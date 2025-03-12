#include "server.h"
#include <chrono>

namespace warpspeed {

HFTServiceImpl::HFTServiceImpl()
    : matching_engine_(
        [this](const Trade& trade) { on_trade(trade); },
        [this](const Quote& quote) { on_quote(quote); }
    )
{
    matching_engine_.start();
}
HFTServiceImpl::~HFTServiceImpl() {
    matching_engine_.stop();
}
grpc::Status HFTServiceImpl::CancelOrder(grpc::ServerContext* context, const warpspeed::CancelRequest* request, warpspeed::CancelResponse* response) {
    std::cout << "[Server] Received cancel request for Order ID: " << request->order_id() << std::endl;
    

    bool success = matching_engine_.cancel_order(request->order_id());

    if (success) {
        response->set_status("SUCCESS: Order cancelled.");
    } else {
        response->set_status("FAILED: Order ID not found or already filled.");
    }

    return grpc::Status::OK;
}
grpc::Status HFTServiceImpl::SubmitOrder(
    grpc::ServerContext* context,
    const OrderRequest* request,
    OrderResponse* response) {
    
    const auto& grpc_order = request->order();

    Order order;
    order.set_order_id(grpc_order.order_id());
    order.set_price(grpc_order.price());
    order.set_quantity(grpc_order.quantity());
    order.set_side(grpc_order.side());

    try {
        matching_engine_.add_order(order);
        response->set_status("SUCCESS");
        response->set_order_id(order.order_id());
        return grpc::Status::OK;
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status HFTServiceImpl::StreamMarketData(
    grpc::ServerContext* context,
    const MarketDataRequest* request,
    grpc::ServerWriter<MarketData>* writer) {
    
    const std::string& instrument = request->instrument();
    
    {
        std::lock_guard<std::mutex> lock(streams_mutex_);
        market_data_streams_[instrument].push_back(writer);
    }

    while (!context->IsCancelled()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    {
        std::lock_guard<std::mutex> lock(streams_mutex_);
        auto& streams = market_data_streams_[instrument];
        streams.erase(
            std::remove(streams.begin(), streams.end(), writer),
            streams.end()
        );
        if (streams.empty()) {
            market_data_streams_.erase(instrument);
        }
    }

    return grpc::Status::OK;
}

void HFTServiceImpl::on_trade(const Trade& trade) {
    MarketData market_data;
    market_data.set_timestamp(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()
    );

    auto* trade_data = market_data.mutable_trade();
    trade_data->set_price(trade.price());
    trade_data->set_quantity(trade.quantity());

    broadcast_market_data("", market_data); 
}

void HFTServiceImpl::on_quote(const Quote& quote) {
    MarketData market_data;
    market_data.set_timestamp(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()
    );

    auto* quote_data = market_data.mutable_quote();
    quote_data->set_bid_price(quote.bid_price());
    quote_data->set_ask_price(quote.ask_price());
    quote_data->set_bid_quantity(quote.bid_quantity());
    quote_data->set_ask_quantity(quote.ask_quantity());

    broadcast_market_data("", market_data); 
}

void HFTServiceImpl::broadcast_market_data(
    const std::string& instrument,
    const MarketData& data) {
    
    std::lock_guard<std::mutex> lock(streams_mutex_);
    
    for (const auto& [stream_instrument, streams] : market_data_streams_) {
        if (instrument.empty() || stream_instrument == instrument) {
            for (auto* writer : streams) {
                writer->Write(data);
            }
        }
    }
}

HFTServer::HFTServer(const std::string& address) {
    service_ = std::make_unique<HFTServiceImpl>();
    
    grpc::ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(service_.get());
    
    server_ = builder.BuildAndStart();
}

void HFTServer::start() {
    server_->Wait();
}

void HFTServer::stop() {
    server_->Shutdown();
}

} 