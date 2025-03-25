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

using namespace std;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

namespace tradeflow {

const int64_t TICK_SIZE = 100;  // 1.00 = 100 ticks

Price doubleToPrice(double price) {
    return static_cast<Price>(price * TICK_SIZE);
}

double priceToDouble(Price price) {
    return static_cast<double>(price) / TICK_SIZE;
}

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
        try {
            if (request->quantity() <= 0) {
                response->set_status("REJECTED");
                response->set_message("Quantity must be positive");
                return Status::OK;
            }
            if (request->price() <= 0) {
                response->set_status("REJECTED");
                response->set_message("Price must be positive");
                return Status::OK;
            }
            if (request->side() != "BUY" && request->side() != "SELL") {
                response->set_status("REJECTED");
                response->set_message("Side must be BUY or SELL");
                return Status::OK;
            }
            if (request->symbol().empty()) {
                response->set_status("REJECTED");
                response->set_message("Symbol is required");
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

            // Trigger matching
            matchers_[request->symbol()]->match(order_book);

            return Status::OK;
        } catch (const exception& e) {
            response->set_status("REJECTED");
            response->set_message(string("Error: ") + e.what());
            return Status::OK;
        }
    }

    Status GetOrderBook(ServerContext* context, const tradeflow::order::GetOrderBookRequest* request,
                        tradeflow::order::GetOrderBookResponse* response) override {
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
            } else {
                response->set_status("NOT_FOUND");
                response->set_message("Order not found");
            }
            return Status::OK;
        } catch (const exception& e) {
            response->set_status("ERROR");
            response->set_message(string("Error: ") + e.what());
            return Status::OK;
        }
    }

    Status ModifyOrder(ServerContext* context, const tradeflow::order::ModifyOrderRequest* request,
                       tradeflow::order::ModifyOrderResponse* response) override {
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
            } else {
                response->set_status("NOT_FOUND");
                response->set_message("Order not found");
            }
            return Status::OK;
        } catch (const exception& e) {
            response->set_status("ERROR");
            response->set_message(string("Error: ") + e.what());
            return Status::OK;
        }
    }

    Status SubscribeTrades(ServerContext* context, const tradeflow::order::SubscribeTradesRequest* request,
                           ServerWriter<tradeflow::order::TradeUpdate>* writer) override {
        auto sub = make_shared<Subscriber>();
        {
            lock_guard<mutex> lock(subscribers_mutex_);
            trade_subscribers_[request->symbol()].push_back(sub);
        }

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

        return Status::OK;
    }
};

} // namespace tradeflow

void RunServer() {
    string server_address("0.0.0.0:50051");
    tradeflow::OrderServiceImpl service;

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
