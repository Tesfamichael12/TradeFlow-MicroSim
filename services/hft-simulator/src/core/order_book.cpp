#include "order_book.h"

namespace warpspeed {

void OrderBook::add_order(const Order& order) {
    if (order.side() == Side::BUY) {
        auto& price_level = buy_orders[order.price()];
        order_lookup[order.order_id()] = std::make_pair(order.price(), price_level.size());
        price_level.push_back(order);
    } else {
        auto& price_level = sell_orders[order.price()];
        order_lookup[order.order_id()] = std::make_pair(order.price(), price_level.size());
        price_level.push_back(order);
    }
}

bool OrderBook::cancel_order(const std::string& order_id) {
    auto it = order_lookup.find(order_id);
    if (it == order_lookup.end()) {
        return false;
    }

    double price = it->second.first;

    auto buy_it = buy_orders.find(price);
    if (buy_it != buy_orders.end()) {
        if (remove_order_from_level(order_id, price, Side::BUY)) {
            remove_empty_level(price, Side::BUY);
            return true;
        }
    }

    auto sell_it = sell_orders.find(price);
    if (sell_it != sell_orders.end()) {
        if (remove_order_from_level(order_id, price, Side::SELL)) {
            remove_empty_level(price, Side::SELL);
            return true;
        }
    }

    return false;
}

std::vector<Trade> OrderBook::match_orders() {
    std::vector<Trade> trades;

    while (!buy_orders.empty() && !sell_orders.empty()) {
        auto best_buy = buy_orders.begin();
        auto best_sell = sell_orders.begin();

        if (best_buy->first < best_sell->first) {
            break; 
        }

        auto& buy_orders_at_level = best_buy->second;
        auto& sell_orders_at_level = best_sell->second;

        if (buy_orders_at_level.empty() || sell_orders_at_level.empty()) {
            break;
        }

        // Match at sell price (price-time priority)
        Order& buy = buy_orders_at_level.front();
        Order& sell = sell_orders_at_level.front();

        int64_t trade_quantity = std::min(buy.quantity(), sell.quantity());

        Trade trade;
        trade.set_price(best_sell->first);
        trade.set_quantity(trade_quantity);
        trades.push_back(trade);

        buy.set_quantity(buy.quantity() - trade_quantity);
        sell.set_quantity(sell.quantity() - trade_quantity);

        if (buy.quantity() == 0) {
            order_lookup.erase(buy.order_id());
            buy_orders_at_level.erase(buy_orders_at_level.begin());
        }
        if (sell.quantity() == 0) {
            order_lookup.erase(sell.order_id());
            sell_orders_at_level.erase(sell_orders_at_level.begin());
        }

        if (buy_orders_at_level.empty()) {
            buy_orders.erase(best_buy);
        }
        if (sell_orders_at_level.empty()) {
            sell_orders.erase(best_sell);
        }
    }

    return trades;
}

std::optional<Quote> OrderBook::get_quote() const {
    if (buy_orders.empty() || sell_orders.empty()) {
        return std::nullopt;
    }

    const auto& best_buy = buy_orders.begin();
    const auto& best_sell = sell_orders.begin();

    Quote quote;
    quote.set_bid_price(best_buy->first);
    quote.set_ask_price(best_sell->first);
    quote.set_bid_quantity(best_buy->second.front().quantity());
    quote.set_ask_quantity(best_sell->second.front().quantity());
    return quote;
}

void OrderBook::remove_empty_level(double price, Side side) {
    if (side == Side::BUY) {
        auto it = buy_orders.find(price);
        if (it != buy_orders.end() && it->second.empty()) {
            buy_orders.erase(it);
        }
    } else {
        auto it = sell_orders.find(price);
        if (it != sell_orders.end() && it->second.empty()) {
            sell_orders.erase(it);
        }
    }
}

bool OrderBook::remove_order_from_level(const std::string& order_id, double price, Side side) {
    auto& orders = (side == Side::BUY) ? buy_orders[price] : sell_orders[price];

    auto order_it = std::find_if(orders.begin(), orders.end(),
        [&order_id](const Order& order) { return order.order_id() == order_id; });

    if (order_it != orders.end()) {
        orders.erase(order_it);
        order_lookup.erase(order_id);
        return true;
    }

    return false;
}

} 