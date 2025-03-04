#pragma once

#include <fstream>
#include <string>
#include <mutex>

namespace tradeflow {

class TradeLog {
private:
    std::ofstream log_file_;
    std::mutex mutex_;

public:
    TradeLog(const std::string& filename) {
        log_file_.open(filename, std::ios::app);
        if (!log_file_.is_open()) {
            throw std::runtime_error("Failed to open trade log file");
        }
    }

    ~TradeLog() {
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }

    // Log trade with raw fields; price is in ticks to avoid float conversion here
    void logTradeRaw(int64_t timestamp_ticks, int64_t buy_order_id, int64_t sell_order_id,
                     int64_t price_ticks, int32_t quantity, const std::string& symbol) {
        std::lock_guard<std::mutex> lock(mutex_);
        log_file_ << timestamp_ticks << "," << buy_order_id << "," << sell_order_id << ","
                  << price_ticks << "," << quantity << "," << symbol << std::endl;
    }
};

} // namespace tradeflow
