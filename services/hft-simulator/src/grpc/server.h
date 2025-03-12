#pragma once

#include <grpcpp/grpcpp.h>
#include <memory>
#include <mutex>
#include <unordered_map>
#include "warpspeed.grpc.pb.h"
#include "../core/matching_engine.h"

namespace warpspeed {

class HFTServiceImpl final : public HFTService::Service {
public:
    explicit HFTServiceImpl();
    ~HFTServiceImpl() override;

    grpc::Status SubmitOrder(
        grpc::ServerContext* context,
        const OrderRequest* request,
        OrderResponse* response) override;
    grpc::Status CancelOrder(
        grpc::ServerContext* context,
        const CancelRequest* request, 
        CancelResponse* response) override;

    grpc::Status StreamMarketData(
        grpc::ServerContext* context,
        const MarketDataRequest* request,
        grpc::ServerWriter<MarketData>* writer) override;

private:
    MatchingEngine matching_engine_;
    std::mutex streams_mutex_;
    std::unordered_map<std::string, std::vector<grpc::ServerWriter<MarketData>*>> market_data_streams_;

    void on_trade(const Trade& trade);
    void on_quote(const Quote& quote);
    void broadcast_market_data(const std::string& instrument, const MarketData& data);
};

class HFTServer {
public:
    explicit HFTServer(const std::string& address);
    void start();
    void stop();

private:
    std::unique_ptr<grpc::Server> server_;
    std::unique_ptr<HFTServiceImpl> service_;
};

} 