#pragma once

#include <map>
#include <vector>
#include <string>
#include <optional>
#include <unordered_map>
#include <cstdint>
#include <algorithm>
#include "warpspeed.pb.h"

namespace warpspeed {

class OrderBook {
public:
    void add_order(const Order& order);
    bool cancel_order(const std::string& order_id);
    std::vector<Trade> match_orders();
    std::optional<Quote> get_quote() const;

private:
    std::map<double, std::vector<Order>, std::greater<double>> buy_orders;
    std::map<double, std::vector<Order>> sell_orders;
    std::unordered_map<std::string, std::pair<double, size_t>> order_lookup;

    void remove_empty_level(double price, Side side);
    bool remove_order_from_level(const std::string& order_id, double price, Side side);
};

}