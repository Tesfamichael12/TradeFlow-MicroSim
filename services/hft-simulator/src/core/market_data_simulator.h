#pragma once

#include <functional>
#include <thread>
#include <atomic>
#include "order_book.h"

namespace warpspeed {

class MarketDataSimulator {
public:
    explicit MarketDataSimulator(const OrderBook& book);
    ~MarketDataSimulator();

    std::function<void(const Quote&)> on_quote_update;

    void start();
    void stop();

private:
    const OrderBook& order_book_;
    std::atomic<bool> running_{false};
    std::unique_ptr<std::thread> simulation_thread_;

    void run_simulation_loop();
};

} 