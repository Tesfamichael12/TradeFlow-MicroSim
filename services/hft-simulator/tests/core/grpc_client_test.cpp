#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <grpcpp/grpcpp.h>
#include "warpspeed.grpc.pb.h"
#include "warpspeed.pb.h"

// Shared state to verify that the trade was seen by the listener thread
std::atomic<bool> trade_seen(false);

void submit_order(warpspeed::HFTService::Stub& stub, const std::string& id, double price, int qty, warpspeed::Side side) {
    warpspeed::Order order;
    order.set_order_id(id);
    order.set_instrument("BTC/USD");
    order.set_price(price);
    order.set_quantity(qty);
    order.set_side(side);

    warpspeed::OrderRequest request;
    *request.mutable_order() = order;

    warpspeed::OrderResponse response;
    grpc::ClientContext context;

    grpc::Status status = stub.SubmitOrder(&context, request, &response);

    if (status.ok()) {
        std::cout << "[Client] ==> Submitted Order: " << response.order_id() << " | Status: " << response.status() << std::endl;
    } else {
        std::cout << "[Client] ==> SubmitOrder gRPC error: " << status.error_message() << std::endl;
    }
}

void cancel_order(warpspeed::HFTService::Stub& stub, const std::string& id) {
    warpspeed::CancelRequest request;
    request.set_order_id(id);

    warpspeed::CancelResponse response;
    grpc::ClientContext context;

    grpc::Status status = stub.CancelOrder(&context, request, &response);

    if (status.ok()) {
        std::cout << "[Client] ==> Sent Cancel Request for: " << id << " | Status: " << response.status() << std::endl;
    } else {
        std::cout << "[Client] ==> CancelOrder gRPC error: " << status.error_message() << std::endl;
    }
}


// This function runs in a separate thread to listen for market data
void listen_for_market_data(warpspeed::HFTService::Stub& stub) {
    warpspeed::MarketDataRequest request;
    request.set_instrument("BTC/USD");

    grpc::ClientContext context;
    std::unique_ptr<grpc::ClientReader<warpspeed::MarketData>> reader(stub.StreamMarketData(&context, request));

    warpspeed::MarketData data;
    std::cout << "[Listener] <== Listening for market data..." << std::endl;

    while (reader->Read(&data)) {
        if (data.has_quote()) {
            std::cout << "[Listener] <== Received QUOTE: Bid=" << data.quote().bid_price()
                      << ", Ask=" << data.quote().ask_price() << std::endl;
        }
        if (data.has_trade()) {
            std::cout << "[Listener] <== !!! RECEIVED TRADE: Price=" << data.trade().price()
                      << ", Qty=" << data.trade().quantity() << " !!!" << std::endl;
            trade_seen = true; 
        }
    }
    
    reader->Finish();
}

int main() {
    auto channel = grpc::CreateChannel("localhost:50052", grpc::InsecureChannelCredentials());
    warpspeed::HFTService::Stub stub(channel);

    std::cout << "--- Starting HFT Simulator Test Client ---" << std::endl;
    std::thread listener_thread(listen_for_market_data, std::ref(stub));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "\n--- TEST CASE 1: Simulating a Full Trade ---" << std::endl;
    submit_order(stub, "SELL_ORDER_1", 101.50, 10, warpspeed::SELL);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    submit_order(stub, "BUY_ORDER_1", 101.50, 10, warpspeed::BUY);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    if (trade_seen) {
        std::cout << "[Main] ==> VERIFICATION SUCCESS: Trade was successfully broadcast and received." << std::endl;
    } else {
        std::cout << "[Main] ==> VERIFICATION FAILED: No trade was received by the listener." << std::endl;
    }    
    std::cout << "\n--- TEST CASE 2: Simulating Order Cancellation ---" << std::endl;

    submit_order(stub, "CANCEL_ME_1", 99.0, 5, warpspeed::BUY);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    cancel_order(stub, "CANCEL_ME_1");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "[Main] ==> Assumed cancellation was processed. Check server logs for confirmation." << std::endl;
    
    std::cout << "\n--- Test Client Finished ---" << std::endl;
    listener_thread.detach(); 
    return 0;
}