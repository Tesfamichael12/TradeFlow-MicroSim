#pragma once

#include "OrderBook.hpp"

namespace tradeflow {

class Matcher {
public:
    void match(OrderBook& orderBook) {
        orderBook.triggerMatching();
    }
};

} // namespace tradeflow
