#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "../../../services/order-matching-engine/include/order_matching/OrderBook.hpp"
#include "../../../services/order-matching-engine/include/order_matching/Order.hpp"

using json = nlohmann::json;
using namespace tradeflow;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: replay_runner <replay.json>" << std::endl;
        return 2;
    }
    std::ifstream in(argv[1]);
    if (!in) { std::cerr << "Cannot open " << argv[1] << std::endl; return 3; }
    json j; in >> j;
    OrderBook ob("REPLAY");
    if (!j.is_object()) {
        std::cerr << "Invalid replay file format: expected JSON object" << std::endl;
        return 4;
    }
    if (!j.contains("events") || !j["events"].is_array()) {
        std::cerr << "Replay JSON missing 'events' array" << std::endl;
        return 5;
    }
    for (auto& ev : j["events"]) {
        std::string type = ev.value("type", "");
        if (type == "add") {
            int64_t id = ev.value("id", 0);
            bool is_buy = ev.value("is_buy", true);
            int qty = ev.value("qty", 0);
            int price = ev.value("price", 0);
            ob.addOrder(id, is_buy, qty, price, "replay");
        } else if (type == "cancel") {
            int64_t id = ev.value("id", 0);
            ob.cancelOrder(id);
        } else if (type == "modify") {
            int64_t id = ev.value("id", 0);
            int qty = ev.value("qty", 0);
            int price = ev.value("price", 0);
            ob.modifyOrder(id, qty, price);
        } else if (type == "match") {
            ob.triggerMatching();
        } else {
            // unknown event type; ignore or log
        }
    }
    std::cout << "Replay finished" << std::endl;
    return 0;
}
