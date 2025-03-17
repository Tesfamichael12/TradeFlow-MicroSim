#include "market_data_simulator.h"
#include <chrono>

namespace warpspeed {

MarketDataSimulator::MarketDataSimulator(const OrderBook& book)
    : order_book_(book) {}

MarketDataSimulator::~MarketDataSimulator() {
    stop();
}

void MarketDataSimulator::start() {
    if (!running_) {
        running_ = true;
        simulation_thread_ = std::make_unique<std::thread>(
            &MarketDataSimulator::run_simulation_loop, this);
    }
}

void MarketDataSimulator::stop() {
    running_ = false;
    if (simulation_thread_ && simulation_thread_->joinable()) {
        simulation_thread_->join();
    }
}

void MarketDataSimulator::run_simulation_loop() {
    using namespace std::chrono_literals;
    
    while (running_) {
        if (auto quote = order_book_.get_quote()) {
            if (on_quote_update) {
                on_quote_update(*quote);
            }
        }
        
        // Sleep for a small duration to prevent CPU spinning
        std::this_thread::sleep_for(100us);
    }
}

}