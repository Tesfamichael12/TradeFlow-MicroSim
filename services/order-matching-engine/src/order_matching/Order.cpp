#include "order_matching/Order.hpp"
#include <chrono>
#include <string>

using namespace std;

namespace tradeflow {

Order::Order()
    : id(0), is_buy(false), quantity(0), price(0), timestamp(chrono::system_clock::now()), client_id(), symbol() {}

Order::Order(OrderId id_, bool is_buy_, Quantity quantity_, Price price_,
             const string& client_id_, const string& symbol_)
    : id(id_), is_buy(is_buy_), quantity(quantity_), price(price_), timestamp(chrono::system_clock::now()), client_id(client_id_), symbol(symbol_) {}

} // namespace tradeflow
