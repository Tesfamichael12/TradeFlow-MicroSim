#include <cassert>
#include <iostream>
#include "order_matching/OrderBook.hpp"

using namespace tradeflow;
using namespace std;

int main() {
    // Basic smoke test for OrderBook: add orders and trigger matching
    OrderBook ob("TEST", MatchingMode::PRICE_TIME_PRIORITY);

    // Add buy at 150.00 (ticks: 15000) quantity 100
    ob.addOrder(1, true, 100, 15000, "clientA");
    // Add sell at 150.00 quantity 50
    ob.addOrder(2, false, 50, 15000, "clientB");

    bool matched = false;
    ob.setTradeCallback([&](const Trade& t) {
        cout << "Test Trade: " << t.quantity << " @ " << t.price << " between " << t.buy_order_id << " and " << t.sell_order_id << "\n";
        matched = true;
    });

    ob.triggerMatching();

    assert(matched && "Expected a trade to occur");

    // After match, buy should have remaining 50
    auto bids = ob.getBidLevels();
    assert(!bids.empty());
    assert(bids.begin()->second == 50);

    cout << "OrderBook unit test passed" << endl;
    return 0;
}
