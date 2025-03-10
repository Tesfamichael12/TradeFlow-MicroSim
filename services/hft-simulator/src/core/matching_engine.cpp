#include "matching_engine.h"
#include <chrono>

namespace warpspeed {

MatchingEngine::MatchingEngine(TradeCallback trade_cb, QuoteCallback quote_cb)
    : on_trade_callback_(std::move(trade_cb))
    , on_quote_callback_(std::move(quote_cb)) {}

MatchingEngine::~MatchingEngine() {
    stop();
}

void MatchingEngine::add_order(const Order& order) {
    order_book_.add_order(order);
    process_trades(order_book_.match_orders());
    publish_quote();
}

bool MatchingEngine::cancel_order(const std::string& order_id) {
    bool result = order_book_.cancel_order(order_id);
    if (result) {
        publish_quote();
    }
    return result;
}

void MatchingEngine::start() {
    if (!running_) {
        running_ = true;
        matching_thread_ = std::make_unique<std::thread>(&MatchingEngine::matching_loop, this);
    }
}

void MatchingEngine::stop() {
    running_ = false;
    if (matching_thread_ && matching_thread_->joinable()) {
        matching_thread_->join();
    }
}

void MatchingEngine::matching_loop() {
    using namespace std::chrono_literals;
    
    while (running_) {
        process_trades(order_book_.match_orders());
        publish_quote();
        
        // Sleep for a small duration to prevent CPU spinning
        std::this_thread::sleep_for(100us);
    }
}

void MatchingEngine::process_trades(const std::vector<Trade>& trades) {
    if (!on_trade_callback_) {
        return;
    }

    for (const auto& trade : trades) {
        on_trade_callback_(trade);
    }
}

void MatchingEngine::publish_quote() {
    if (!on_quote_callback_) {
        return;
    }

    if (auto quote = order_book_.get_quote()) {
        on_quote_callback_(*quote);
    }
}

}