#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include "../../include/order_matching/OrderBook.hpp"

using json = nlohmann::json;
using namespace tradeflow;
using namespace std;

TEST(ReplayTest, SimpleReplayEndsEmpty) {
    string replay_path = string(TEST_SRCDIR) + "/tests/data/replays/simple_replay.json";
    ifstream in(replay_path);
    ASSERT_TRUE(in.good()) << "Could not open replay file: " << replay_path;
    json j; in >> j;
    OrderBook ob("TEST_REPLAY");
    for (auto& ev : j["events"]) {
        std::string type = ev["type"];
        if (type == "add") {
            int64_t id = ev["id"];
            bool is_buy = ev["is_buy"];
            int qty = ev["qty"];
            int price = ev["price"];
            ob.addOrder(id, is_buy, qty, price, "replay");
        } else if (type == "cancel") {
            int64_t id = ev["id"];
            ob.cancelOrder(id);
        } else if (type == "modify") {
            int64_t id = ev["id"];
            int qty = ev["qty"];
            int price = ev["price"];
            ob.modifyOrder(id, qty, price);
        } else if (type == "match") {
            ob.triggerMatching();
        }
    }
    // After the simple replay the book should be empty
    auto bids = ob.getBidLevels();
    auto asks = ob.getAskLevels();
    EXPECT_TRUE(bids.empty());
    EXPECT_TRUE(asks.empty());
}
