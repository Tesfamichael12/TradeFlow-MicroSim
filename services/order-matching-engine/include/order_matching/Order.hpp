#pragma once

#include <chrono>
#include <string>
#include <memory>

namespace tradeflow {

using Price = int64_t;  // Price in ticks (e.g., 100.00 = 10000 ticks if tick_size=0.01)
using Quantity = int32_t;
using OrderId = int64_t;
using Timestamp = std::chrono::system_clock::time_point;

struct Order {
    OrderId id;
    bool is_buy;
    Quantity quantity;
    Price price;
    Timestamp timestamp;
    std::string client_id;
    std::string symbol;

    Order();
    Order(OrderId id_, bool is_buy_, Quantity quantity_, Price price_,
          const std::string& client_id_, const std::string& symbol_);
};

} // namespace tradeflow
