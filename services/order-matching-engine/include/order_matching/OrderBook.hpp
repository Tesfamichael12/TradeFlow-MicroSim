#pragma once

#include <deque>
#include <unordered_map>
#include <map>
#include <memory>
#include <functional>
#include <vector>
#include <shared_mutex>
#include "Order.hpp"
#include "TradeLog.hpp"

namespace tradeflow {

enum class MatchingMode {
    PRICE_TIME_PRIORITY,
    PRO_RATA,
    CALL_AUCTION
};

struct Trade {
    OrderId buy_order_id;
    OrderId sell_order_id;
    Price price;
    Quantity quantity;
    std::string symbol;
    Timestamp timestamp;
};

using TradeCallback = std::function<void(const Trade&)>;

struct PriceLevel {
    Price price;
    Quantity total_quantity;
    std::deque<Order*> orders;  // FIFO queue

    PriceLevel(Price p) : price(p), total_quantity(0) {}
};

class OrderBook {
private:
    std::unordered_map<OrderId, std::unique_ptr<Order>> order_map_;
    std::map<Price, std::unique_ptr<PriceLevel>> bid_levels_;  // Buy: higher price first
    std::map<Price, std::unique_ptr<PriceLevel>, std::greater<Price>> ask_levels_;  // Sell: lower price first
    mutable std::shared_mutex mutex_;
    MatchingMode mode_;
    TradeCallback trade_callback_;
    std::string symbol_;
    std::unique_ptr<TradeLog> trade_log_;

    void addToLevel(Order* order);
    void removeFromLevel(Order* order);
    Price getBestBid() const;
    Price getBestAsk() const;
    void matchOrders();
    void matchPriceTime(PriceLevel* bid_level, PriceLevel* ask_level);
    void matchProRata(PriceLevel* bid_level, PriceLevel* ask_level);
    void executeTrade(Order* buy_order, Order* sell_order, Quantity qty, Price px);

public:
    explicit OrderBook(const std::string& symbol, MatchingMode mode = MatchingMode::PRICE_TIME_PRIORITY);
    void setTradeCallback(TradeCallback callback);
    void setTradeLog(std::unique_ptr<TradeLog> log);
    bool addOrder(OrderId id, bool is_buy, Quantity qty, Price px, const std::string& client_id);
    bool cancelOrder(OrderId id);
    bool modifyOrder(OrderId id, Quantity new_qty, Price new_px);
    std::vector<std::pair<Price, Quantity>> getBidLevels() const;
    std::vector<std::pair<Price, Quantity>> getAskLevels() const;
    void triggerMatching();
};

} // namespace tradeflow
