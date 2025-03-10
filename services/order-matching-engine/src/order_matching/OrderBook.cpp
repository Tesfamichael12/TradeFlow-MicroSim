#include "order_matching/OrderBook.hpp"
#include <algorithm>
#include <iostream>

using namespace std;

namespace tradeflow {

OrderBook::OrderBook(const string& symbol, MatchingMode mode)
    : order_map_(), bid_levels_(), ask_levels_(), mutex_(), mode_(mode), trade_callback_(nullptr), symbol_(symbol), trade_log_(nullptr) {}

void OrderBook::setTradeCallback(TradeCallback callback) {
    trade_callback_ = callback;
}

void OrderBook::setTradeLog(unique_ptr<TradeLog> log) {
    trade_log_ = move(log);
}

void OrderBook::addToLevel(Order* order) {
    if (order->is_buy) {
        auto it = bid_levels_.find(order->price);
        if (it == bid_levels_.end()) {
            it = bid_levels_.emplace(order->price, make_unique<PriceLevel>(order->price)).first;
        }
        it->second->orders.push_back(order);
        it->second->total_quantity += order->quantity;
    } else {
        auto it = ask_levels_.find(order->price);
        if (it == ask_levels_.end()) {
            it = ask_levels_.emplace(order->price, make_unique<PriceLevel>(order->price)).first;
        }
        it->second->orders.push_back(order);
        it->second->total_quantity += order->quantity;
    }
}

void OrderBook::removeFromLevel(Order* order) {
    if (order->is_buy) {
        auto it = bid_levels_.find(order->price);
        if (it != bid_levels_.end()) {
            auto& orders = it->second->orders;
            auto order_it = find(orders.begin(), orders.end(), order);
            if (order_it != orders.end()) {
                orders.erase(order_it);
                it->second->total_quantity -= order->quantity;
                if (orders.empty()) {
                    bid_levels_.erase(it);
                }
            }
        }
    } else {
        auto it = ask_levels_.find(order->price);
        if (it != ask_levels_.end()) {
            auto& orders = it->second->orders;
            auto order_it = find(orders.begin(), orders.end(), order);
            if (order_it != orders.end()) {
                orders.erase(order_it);
                it->second->total_quantity -= order->quantity;
                if (orders.empty()) {
                    ask_levels_.erase(it);
                }
            }
        }
    }
}

Price OrderBook::getBestBid() const {
    if (bid_levels_.empty()) return 0;
    return bid_levels_.rbegin()->first;  // Highest price
}

Price OrderBook::getBestAsk() const {
    if (ask_levels_.empty()) return INT64_MAX;
    return ask_levels_.rbegin()->first;  // Lowest price
}

bool OrderBook::addOrder(OrderId id, bool is_buy, Quantity qty, Price px, const string& client_id) {
    unique_lock lock(mutex_);
    auto order = make_unique<Order>(id, is_buy, qty, px, client_id, symbol_);
    Order* order_ptr = order.get();
    order_map_[id] = move(order);
    addToLevel(order_ptr);
    return true;
}

bool OrderBook::cancelOrder(OrderId id) {
    unique_lock lock(mutex_);
    auto it = order_map_.find(id);
    if (it == order_map_.end()) return false;
    removeFromLevel(it->second.get());
    order_map_.erase(it);
    return true;
}

bool OrderBook::modifyOrder(OrderId id, Quantity new_qty, Price new_px) {
    unique_lock lock(mutex_);
    auto it = order_map_.find(id);
    if (it == order_map_.end()) return false;
    Order* order = it->second.get();
    removeFromLevel(order);
    order->quantity = new_qty;
    order->price = new_px;
    addToLevel(order);
    return true;
}

vector<pair<Price, Quantity>> OrderBook::getBidLevels() const {
    shared_lock lock(mutex_);
    vector<pair<Price, Quantity>> levels;
    for (auto it = bid_levels_.rbegin(); it != bid_levels_.rend(); ++it) {
        levels.emplace_back(it->first, it->second->total_quantity);
    }
    return levels;
}

vector<pair<Price, Quantity>> OrderBook::getAskLevels() const {
    shared_lock lock(mutex_);
    vector<pair<Price, Quantity>> levels;
    for (auto it = ask_levels_.rbegin(); it != ask_levels_.rend(); ++it) {
        levels.emplace_back(it->first, it->second->total_quantity);
    }
    return levels;
}

void OrderBook::triggerMatching() {
    unique_lock lock(mutex_);
    matchOrders();
}

void OrderBook::matchOrders() {
    while (!bid_levels_.empty() && !ask_levels_.empty()) {
        Price best_bid = getBestBid();
        Price best_ask = getBestAsk();
        if (best_bid < best_ask) break;

        auto bid_it = bid_levels_.find(best_bid);
        auto ask_it = ask_levels_.find(best_ask);
        if (bid_it == bid_levels_.end() || ask_it == ask_levels_.end()) break;

        PriceLevel* bid_level = bid_it->second.get();
        PriceLevel* ask_level = ask_it->second.get();

        if (mode_ == MatchingMode::PRICE_TIME_PRIORITY) {
            matchPriceTime(bid_level, ask_level);
        } else if (mode_ == MatchingMode::PRO_RATA) {
            matchProRata(bid_level, ask_level);
        }
        // Remove empty levels
        if (bid_level->orders.empty()) bid_levels_.erase(bid_it);
        if (ask_level->orders.empty()) ask_levels_.erase(ask_it);
    }
}

void OrderBook::matchPriceTime(PriceLevel* bid_level, PriceLevel* ask_level) {
    while (!bid_level->orders.empty() && !ask_level->orders.empty()) {
        Order* buy_order = bid_level->orders.front();
        Order* sell_order = ask_level->orders.front();

    Quantity match_qty = min(buy_order->quantity, sell_order->quantity);
    executeTrade(buy_order, sell_order, match_qty, sell_order->price);

    // Maintain level total quantities
    if (bid_level->total_quantity >= match_qty) bid_level->total_quantity -= match_qty;
    else bid_level->total_quantity = 0;
    if (ask_level->total_quantity >= match_qty) ask_level->total_quantity -= match_qty;
    else ask_level->total_quantity = 0;

    buy_order->quantity -= match_qty;
    sell_order->quantity -= match_qty;

        if (buy_order->quantity == 0) {
            bid_level->orders.pop_front();
            order_map_.erase(buy_order->id);
        }
        if (sell_order->quantity == 0) {
            ask_level->orders.pop_front();
            order_map_.erase(sell_order->id);
        }
    }
}

void OrderBook::matchProRata(PriceLevel* bid_level, PriceLevel* ask_level) {
    // Simplified pro-rata: allocate proportionally
    Quantity total_bid_qty = bid_level->total_quantity;
    Quantity total_ask_qty = ask_level->total_quantity;
    Quantity match_qty = min(total_bid_qty, total_ask_qty);

    for (auto* buy_order : bid_level->orders) {
        Quantity alloc = (buy_order->quantity * match_qty) / total_bid_qty;
        if (alloc > 0) {
            // Match with sell orders proportionally
            for (auto* sell_order : ask_level->orders) {
                Quantity sell_alloc = min(alloc, sell_order->quantity);
                if (sell_alloc > 0) {
                    executeTrade(buy_order, sell_order, sell_alloc, sell_order->price);
                    buy_order->quantity -= sell_alloc;
                    sell_order->quantity -= sell_alloc;
                    alloc -= sell_alloc;
                    if (alloc == 0) break;
                }
            }
        }
    }
    // Clean up filled orders
    bid_level->orders.erase(remove_if(bid_level->orders.begin(), bid_level->orders.end(),
                                           [](Order* o) { return o->quantity == 0; }), bid_level->orders.end());
    ask_level->orders.erase(remove_if(ask_level->orders.begin(), ask_level->orders.end(),
                                           [](Order* o) { return o->quantity == 0; }), ask_level->orders.end());
    for (auto* o : bid_level->orders) order_map_.erase(o->id);
    for (auto* o : ask_level->orders) order_map_.erase(o->id);
}

void OrderBook::executeTrade(Order* buy_order, Order* sell_order, Quantity qty, Price px) {
    Trade trade{buy_order->id, sell_order->id, px, qty, symbol_, chrono::system_clock::now()};
    if (trade_callback_) {
        trade_callback_(trade);
    }
    if (trade_log_) {
        auto ts = chrono::duration_cast<chrono::milliseconds>(trade.timestamp.time_since_epoch()).count();
        trade_log_->logTradeRaw(ts, trade.buy_order_id, trade.sell_order_id, trade.price, trade.quantity, trade.symbol);
    }
    cout << "Trade: " << qty << " @ " << px << " between " << buy_order->id << " and " << sell_order->id << endl;
}

} // namespace tradeflow
