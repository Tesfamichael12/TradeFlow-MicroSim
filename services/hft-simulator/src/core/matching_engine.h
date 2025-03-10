#pragma once

#include <memory>
#include <functional>
#include <atomic>
#include <thread>
#include "order_book.h"

namespace warpspeed {

class MatchingEngine {
public:
    using TradeCallback = std::function<void(const Trade&)>;
    using QuoteCallback = std::function<void(const Quote&)>;

    MatchingEngine(TradeCallback trade_cb = nullptr, QuoteCallback quote_cb = nullptr);
    ~MatchingEngine();

    void add_order(const Order& order);
    bool cancel_order(const std::string& order_id);

    void start();
    void stop();

private:
    OrderBook order_book_;
    TradeCallback on_trade_callback_;
    QuoteCallback on_quote_callback_;
    std::atomic<bool> running_{false};
    std::unique_ptr<std::thread> matching_thread_;

    void matching_loop();
    void process_trades(const std::vector<Trade>& trades);
    void publish_quote();
};

} 