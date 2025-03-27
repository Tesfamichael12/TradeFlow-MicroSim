#include <gtest/gtest.h>
#include <atomic>
#include <iostream>
#include "order_matching/OrderBook.hpp"

using namespace tradeflow;

namespace {

TEST(OrderBookTest, MatchesOrdersPriceTime) {
    OrderBook ob("TEST", MatchingMode::PRICE_TIME_PRIORITY);

    // Add initial orders
    ASSERT_TRUE(ob.addOrder(1, true, 100, 15000, "clientA"));
    ASSERT_TRUE(ob.addOrder(2, false, 50, 15000, "clientB"));

    std::atomic<bool> matched{false};
    Trade captured{};
    ob.setTradeCallback([&](const Trade& trade) {
        matched = true;
        captured = trade;
        std::cout << "Test Trade: " << trade.quantity << " @ " << trade.price
                  << " between " << trade.buy_order_id << " and " << trade.sell_order_id << std::endl;
    });

    ob.triggerMatching();

    EXPECT_TRUE(matched.load()) << "Expected a trade to occur";
    EXPECT_EQ(1, captured.buy_order_id);
    EXPECT_EQ(2, captured.sell_order_id);
    EXPECT_EQ(15000, captured.price);
    EXPECT_EQ(50, captured.quantity);

    const auto bids = ob.getBidLevels();
    ASSERT_FALSE(bids.empty());
    EXPECT_EQ(15000, bids.front().first);
    EXPECT_EQ(50, bids.front().second) << "Remaining quantity should reflect partial fill";

    const auto asks = ob.getAskLevels();
    EXPECT_TRUE(asks.empty()) << "Sell side should be cleared after fill";
}

TEST(OrderBookTest, CancelAndModifyOrder) {
    OrderBook ob("TEST", MatchingMode::PRICE_TIME_PRIORITY);

    ASSERT_TRUE(ob.addOrder(10, true, 200, 10100, "buyer"));
    ASSERT_TRUE(ob.addOrder(11, false, 200, 10200, "seller"));

    EXPECT_TRUE(ob.modifyOrder(11, 150, 10100));
    auto asks = ob.getAskLevels();
    ASSERT_FALSE(asks.empty());
    EXPECT_EQ(10100, asks.front().first);
    EXPECT_EQ(150, asks.front().second);

    EXPECT_TRUE(ob.cancelOrder(10));
    auto bids = ob.getBidLevels();
    EXPECT_TRUE(bids.empty());
}

}  // namespace

