#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include <thread>
#include <mutex>
#include <unordered_map>
#include "order_service.grpc.pb.h"
#include "order_matching/OrderBook.hpp"
#include "order_matching/TradeLog.hpp"
#include "order_matching/Matcher.hpp"
#include <condition_variable>
#include <deque>
#include <algorithm>
#include <chrono>
#include <atomic>
#include <sstream>
#ifndef _WIN32
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

using namespace std;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

namespace tradeflow {

const int64_t TICK_SIZE = 100;  // 1.00 = 100 ticks
constexpr int METRICS_PORT = 9464;

std::atomic<uint64_t> metrics_submit_requests{0};
std::atomic<uint64_t> metrics_submit_accepted{0};
std::atomic<uint64_t> metrics_submit_rejected{0};
std::atomic<uint64_t> metrics_submit_errors{0};
std::atomic<uint64_t> metrics_get_orderbook_requests{0};
std::atomic<uint64_t> metrics_cancel_requests{0};
std::atomic<uint64_t> metrics_cancel_success{0};
std::atomic<uint64_t> metrics_cancel_not_found{0};
std::atomic<uint64_t> metrics_cancel_errors{0};
std::atomic<uint64_t> metrics_modify_requests{0};
std::atomic<uint64_t> metrics_modify_success{0};
std::atomic<uint64_t> metrics_modify_not_found{0};
std::atomic<uint64_t> metrics_modify_errors{0};
std::atomic<uint64_t> metrics_trade_updates_published{0};
std::atomic<uint64_t> metrics_trade_quantity_total{0};
std::atomic<long long> metrics_last_trade_timestamp_epoch{0};
std::atomic<uint64_t> metrics_subscribe_requests{0};
std::atomic<int64_t> metrics_active_trade_subscriptions{0};

std::string CollectMetricsSnapshot();
void MetricsHttpServer();

Price doubleToPrice(double price) {
    return static_cast<Price>(price * TICK_SIZE);
}

double priceToDouble(Price price) {
    return static_cast<double>(price) / TICK_SIZE;
}

std::string CollectMetricsSnapshot() {
    std::ostringstream oss;
    oss << "# HELP tradeflow_order_service_submit_requests_total Total SubmitOrder RPCs received" << '\n';
    oss << "# TYPE tradeflow_order_service_submit_requests_total counter" << '\n';
    oss << "tradeflow_order_service_submit_requests_total " << metrics_submit_requests.load() << '\n';

    oss << "# HELP tradeflow_order_service_submit_accepted_total Successfully accepted submit requests" << '\n';
    oss << "# TYPE tradeflow_order_service_submit_accepted_total counter" << '\n';
    oss << "tradeflow_order_service_submit_accepted_total " << metrics_submit_accepted.load() << '\n';

    oss << "# HELP tradeflow_order_service_submit_rejected_total Submit requests rejected due to validation" << '\n';
    oss << "# TYPE tradeflow_order_service_submit_rejected_total counter" << '\n';
    oss << "tradeflow_order_service_submit_rejected_total " << metrics_submit_rejected.load() << '\n';

    oss << "# HELP tradeflow_order_service_submit_errors_total Submit requests that triggered internal errors" << '\n';
    oss << "# TYPE tradeflow_order_service_submit_errors_total counter" << '\n';
    oss << "tradeflow_order_service_submit_errors_total " << metrics_submit_errors.load() << '\n';

    oss << "# HELP tradeflow_order_service_get_orderbook_requests_total Total GetOrderBook RPCs" << '\n';
    oss << "# TYPE tradeflow_order_service_get_orderbook_requests_total counter" << '\n';
    oss << "tradeflow_order_service_get_orderbook_requests_total " << metrics_get_orderbook_requests.load() << '\n';

    oss << "# HELP tradeflow_order_service_cancel_requests_total Total CancelOrder RPCs" << '\n';
    oss << "# TYPE tradeflow_order_service_cancel_requests_total counter" << '\n';
    oss << "tradeflow_order_service_cancel_requests_total " << metrics_cancel_requests.load() << '\n';

    oss << "# HELP tradeflow_order_service_cancel_success_total CancelOrder RPCs that cancelled an order" << '\n';
    oss << "# TYPE tradeflow_order_service_cancel_success_total counter" << '\n';
    oss << "tradeflow_order_service_cancel_success_total " << metrics_cancel_success.load() << '\n';

    oss << "# HELP tradeflow_order_service_cancel_not_found_total CancelOrder RPCs where order was not found" << '\n';
    oss << "# TYPE tradeflow_order_service_cancel_not_found_total counter" << '\n';
    oss << "tradeflow_order_service_cancel_not_found_total " << metrics_cancel_not_found.load() << '\n';

    oss << "# HELP tradeflow_order_service_cancel_errors_total CancelOrder RPCs that triggered internal errors" << '\n';
    oss << "# TYPE tradeflow_order_service_cancel_errors_total counter" << '\n';
    oss << "tradeflow_order_service_cancel_errors_total " << metrics_cancel_errors.load() << '\n';

    oss << "# HELP tradeflow_order_service_modify_requests_total Total ModifyOrder RPCs" << '\n';
    oss << "# TYPE tradeflow_order_service_modify_requests_total counter" << '\n';
    oss << "tradeflow_order_service_modify_requests_total " << metrics_modify_requests.load() << '\n';

    oss << "# HELP tradeflow_order_service_modify_success_total ModifyOrder RPCs that modified an order" << '\n';
    oss << "# TYPE tradeflow_order_service_modify_success_total counter" << '\n';
    oss << "tradeflow_order_service_modify_success_total " << metrics_modify_success.load() << '\n';

    oss << "# HELP tradeflow_order_service_modify_not_found_total ModifyOrder RPCs where order was not found" << '\n';
    oss << "# TYPE tradeflow_order_service_modify_not_found_total counter" << '\n';
    oss << "tradeflow_order_service_modify_not_found_total " << metrics_modify_not_found.load() << '\n';

    oss << "# HELP tradeflow_order_service_modify_errors_total ModifyOrder RPCs that triggered internal errors" << '\n';
    oss << "# TYPE tradeflow_order_service_modify_errors_total counter" << '\n';
    oss << "tradeflow_order_service_modify_errors_total " << metrics_modify_errors.load() << '\n';

    oss << "# HELP tradeflow_order_service_trade_updates_total Trade updates published to subscribers" << '\n';
    oss << "# TYPE tradeflow_order_service_trade_updates_total counter" << '\n';
    oss << "tradeflow_order_service_trade_updates_total " << metrics_trade_updates_published.load() << '\n';

    oss << "# HELP tradeflow_order_service_trade_quantity_total Cumulative filled quantity across trades" << '\n';
    oss << "# TYPE tradeflow_order_service_trade_quantity_total counter" << '\n';
    oss << "tradeflow_order_service_trade_quantity_total " << metrics_trade_quantity_total.load() << '\n';

    long long last_trade = metrics_last_trade_timestamp_epoch.load();
    if (last_trade > 0) {
        oss << "# HELP tradeflow_order_service_last_trade_timestamp_seconds Epoch seconds of the most recent trade" << '\n';
        oss << "# TYPE tradeflow_order_service_last_trade_timestamp_seconds gauge" << '\n';
        oss << "tradeflow_order_service_last_trade_timestamp_seconds " << last_trade << '\n';
    }

    oss << "# HELP tradeflow_order_service_subscribe_requests_total Total SubscribeTrades RPCs" << '\n';
    oss << "# TYPE tradeflow_order_service_subscribe_requests_total counter" << '\n';
    oss << "tradeflow_order_service_subscribe_requests_total " << metrics_subscribe_requests.load() << '\n';

    oss << "# HELP tradeflow_order_service_active_trade_subscriptions Active trade streaming subscriptions" << '\n';
    oss << "# TYPE tradeflow_order_service_active_trade_subscriptions gauge" << '\n';
    oss << "tradeflow_order_service_active_trade_subscriptions " << metrics_active_trade_subscriptions.load() << '\n';

    return oss.str();
}

#ifndef _WIN32
void MetricsHttpServer() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "[metrics] failed to create socket" << std::endl;
        return;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(METRICS_PORT);

    if (bind(server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        std::cerr << "[metrics] failed to bind socket on port " << METRICS_PORT << std::endl;
        close(server_fd);
        return;
    }

    if (listen(server_fd, 16) < 0) {
        std::cerr << "[metrics] failed to listen on port " << METRICS_PORT << std::endl;
        close(server_fd);
        return;
    }

    std::cout << "[metrics] Prometheus exporter listening on 0.0.0.0:" << METRICS_PORT << std::endl;

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            continue;
        }

        char buffer[1024];
        (void)read(client_fd, buffer, sizeof(buffer));

        std::string body = CollectMetricsSnapshot();
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: text/plain; version=0.0.4\r\n";
        response << "Content-Length: " << body.size() << "\r\n";
        response << "Connection: close\r\n\r\n";
        response << body;

        std::string payload = response.str();
        (void)send(client_fd, payload.data(), payload.size(), 0);
        close(client_fd);
    }
}
#else
void MetricsHttpServer() {
    std::cerr << "[metrics] Prometheus exporter disabled on Windows builds" << std::endl;
}
#endif

// Global variables for order books and subscribers
unordered_map<string, unique_ptr<OrderBook>> order_books_;
unordered_map<string, unique_ptr<Matcher>> matchers_;
mutex order_books_mutex_;
OrderId next_order_id_ = 1;
mutex id_mutex_;

// For streaming trades: per-subscriber queue + condition variable
struct Subscriber {
    mutex m;
    condition_variable cv;
    deque<tradeflow::order::TradeUpdate> q;
    bool active = true;
};

unordered_map<string, vector<shared_ptr<Subscriber>>> trade_subscribers_;
mutex subscribers_mutex_;

void publishTrade(const Trade& trade) {
    tradeflow::order::TradeUpdate update;
    update.set_buy_order_id(to_string(trade.buy_order_id));
    update.set_sell_order_id(to_string(trade.sell_order_id));
    update.set_price(priceToDouble(trade.price));
    update.set_quantity(trade.quantity);
    update.set_symbol(trade.symbol);
    auto time_t = chrono::system_clock::to_time_t(trade.timestamp);
    update.set_timestamp(ctime(&time_t));

    metrics_trade_updates_published.fetch_add(1, std::memory_order_relaxed);
    metrics_trade_quantity_total.fetch_add(static_cast<uint64_t>(trade.quantity), std::memory_order_relaxed);
    auto epoch_seconds = chrono::duration_cast<chrono::seconds>(trade.timestamp.time_since_epoch()).count();
    metrics_last_trade_timestamp_epoch.store(epoch_seconds, std::memory_order_relaxed);

    lock_guard<mutex> lock(subscribers_mutex_);
    auto it = trade_subscribers_.find(trade.symbol);
    if (it != trade_subscribers_.end()) {
        for (auto& sub : it->second) {
            lock_guard<mutex> lk(sub->m);
            sub->q.push_back(update);
            sub->cv.notify_one();
        }
    }
}

OrderBook& getOrderBook(const string& symbol) {
    lock_guard<mutex> lock(order_books_mutex_);
    if (order_books_.find(symbol) == order_books_.end()) {
        order_books_[symbol] = make_unique<OrderBook>(symbol, MatchingMode::PRICE_TIME_PRIORITY);
        matchers_[symbol] = make_unique<Matcher>();
        order_books_[symbol]->setTradeCallback([&](const Trade& trade) { publishTrade(trade); });
        order_books_[symbol]->setTradeLog(make_unique<TradeLog>(symbol + "_trades.log"));
    }
    return *order_books_[symbol];
}

OrderId getNextOrderId() {
    lock_guard<mutex> lock(id_mutex_);
    return next_order_id_++;
}

class OrderServiceImpl final : public tradeflow::order::OrderService::Service {
public:
    Status SubmitOrder(ServerContext* context, const tradeflow::order::SubmitOrderRequest* request,
                       tradeflow::order::SubmitOrderResponse* response) override {
        metrics_submit_requests.fetch_add(1, std::memory_order_relaxed);
        try {
            if (request->quantity() <= 0) {
                response->set_status("REJECTED");
                response->set_message("Quantity must be positive");
                metrics_submit_rejected.fetch_add(1, std::memory_order_relaxed);
                return Status::OK;
            }
            if (request->price() <= 0) {
                response->set_status("REJECTED");
                response->set_message("Price must be positive");
                metrics_submit_rejected.fetch_add(1, std::memory_order_relaxed);
                return Status::OK;
            }
            if (request->side() != "BUY" && request->side() != "SELL") {
                response->set_status("REJECTED");
                response->set_message("Side must be BUY or SELL");
                metrics_submit_rejected.fetch_add(1, std::memory_order_relaxed);
                return Status::OK;
            }
            if (request->symbol().empty()) {
                response->set_status("REJECTED");
                response->set_message("Symbol is required");
                metrics_submit_rejected.fetch_add(1, std::memory_order_relaxed);
                return Status::OK;
            }

            bool is_buy = (request->side() == "BUY");
            OrderId order_id = getNextOrderId();
            Price price = doubleToPrice(request->price());
            OrderBook& order_book = getOrderBook(request->symbol());
            order_book.addOrder(order_id, is_buy, request->quantity(), price, request->client_id());

            response->set_order_id(to_string(order_id));
            response->set_status("ACCEPTED");
            response->set_message("Order submitted successfully");

            metrics_submit_accepted.fetch_add(1, std::memory_order_relaxed);

            // Trigger matching
            matchers_[request->symbol()]->match(order_book);

            return Status::OK;
        } catch (const exception& e) {
            response->set_status("REJECTED");
            response->set_message(string("Error: ") + e.what());
            metrics_submit_errors.fetch_add(1, std::memory_order_relaxed);
            return Status::OK;
        }
    }

    Status GetOrderBook(ServerContext* context, const tradeflow::order::GetOrderBookRequest* request,
                        tradeflow::order::GetOrderBookResponse* response) override {
        metrics_get_orderbook_requests.fetch_add(1, std::memory_order_relaxed);
        OrderBook& order_book = getOrderBook(request->symbol());
        auto bids = order_book.getBidLevels();
        auto asks = order_book.getAskLevels();

        for (const auto& level : bids) {
            auto* entry = response->add_bids();
            entry->set_price(priceToDouble(level.first));
            entry->set_quantity(level.second);
        }

        for (const auto& level : asks) {
            auto* entry = response->add_asks();
            entry->set_price(priceToDouble(level.first));
            entry->set_quantity(level.second);
        }

        return Status::OK;
    }

    Status CancelOrder(ServerContext* context, const tradeflow::order::CancelOrderRequest* request,
                       tradeflow::order::CancelOrderResponse* response) override {
        metrics_cancel_requests.fetch_add(1, std::memory_order_relaxed);
        try {
            OrderId order_id = stoll(request->order_id());
            // Find the order book for the order
            bool found = false;
            lock_guard<mutex> lock(order_books_mutex_);
            for (auto& pair : order_books_) {
                if (pair.second->cancelOrder(order_id)) {
                    found = true;
                    break;
                }
            }

            if (found) {
                response->set_status("CANCELLED");
                response->set_message("Order cancelled successfully");
                metrics_cancel_success.fetch_add(1, std::memory_order_relaxed);
            } else {
                response->set_status("NOT_FOUND");
                response->set_message("Order not found");
                metrics_cancel_not_found.fetch_add(1, std::memory_order_relaxed);
            }
            return Status::OK;
        } catch (const exception& e) {
            response->set_status("ERROR");
            response->set_message(string("Error: ") + e.what());
            metrics_cancel_errors.fetch_add(1, std::memory_order_relaxed);
            return Status::OK;
        }
    }

    Status ModifyOrder(ServerContext* context, const tradeflow::order::ModifyOrderRequest* request,
                       tradeflow::order::ModifyOrderResponse* response) override {
        metrics_modify_requests.fetch_add(1, std::memory_order_relaxed);
        try {
            OrderId order_id = stoll(request->order_id());
            Price new_price = doubleToPrice(request->new_price());
            bool found = false;
            lock_guard<mutex> lock(order_books_mutex_);
            for (auto& pair : order_books_) {
                if (pair.second->modifyOrder(order_id, request->new_quantity(), new_price)) {
                    found = true;
                    break;
                }
            }

            if (found) {
                response->set_status("MODIFIED");
                response->set_message("Order modified successfully");
                metrics_modify_success.fetch_add(1, std::memory_order_relaxed);
            } else {
                response->set_status("NOT_FOUND");
                response->set_message("Order not found");
                metrics_modify_not_found.fetch_add(1, std::memory_order_relaxed);
            }
            return Status::OK;
        } catch (const exception& e) {
            response->set_status("ERROR");
            response->set_message(string("Error: ") + e.what());
            metrics_modify_errors.fetch_add(1, std::memory_order_relaxed);
            return Status::OK;
        }
    }

    Status SubscribeTrades(ServerContext* context, const tradeflow::order::SubscribeTradesRequest* request,
                           ServerWriter<tradeflow::order::TradeUpdate>* writer) override {
        metrics_subscribe_requests.fetch_add(1, std::memory_order_relaxed);
        auto sub = make_shared<Subscriber>();
        {
            lock_guard<mutex> lock(subscribers_mutex_);
            trade_subscribers_[request->symbol()].push_back(sub);
        }
        metrics_active_trade_subscriptions.fetch_add(1, std::memory_order_relaxed);

        // Loop until client disconnects; write updates from the subscriber queue
        while (true) {
            unique_lock<mutex> lk(sub->m);
            sub->cv.wait(lk, [&] { return !sub->q.empty() || !sub->active || context->IsCancelled(); });

            if (context->IsCancelled()) break;

            while (!sub->q.empty()) {
                auto update = sub->q.front();
                sub->q.pop_front();
                lk.unlock();
                writer->Write(update);
                lk.lock();
            }
        }

        // remove subscriber
        {
            lock_guard<mutex> lock(subscribers_mutex_);
            auto& vec = trade_subscribers_[request->symbol()];
            vec.erase(remove_if(vec.begin(), vec.end(), [&](const shared_ptr<Subscriber>& s) { return s == sub; }), vec.end());
        }

        metrics_active_trade_subscriptions.fetch_sub(1, std::memory_order_relaxed);

        return Status::OK;
    }
};

} // namespace tradeflow

void RunServer() {
    string server_address("0.0.0.0:50051");
    tradeflow::OrderServiceImpl service;

    std::thread metrics_thread(tradeflow::MetricsHttpServer);
    metrics_thread.detach();

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    unique_ptr<Server> server(builder.BuildAndStart());
    cout << "Order Matching Engine Server listening on " << server_address << endl;

    server->Wait();
}

int main(int argc, char** argv) {
    RunServer();
    return 0;
}
