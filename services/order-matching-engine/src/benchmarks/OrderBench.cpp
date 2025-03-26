#include <benchmark/benchmark.h>
#include "../../include/order_matching/Order.hpp"
#include "../../include/order_matching/OrderBook.hpp"

using namespace tradeflow;
using namespace benchmark;

static void BM_AddOrderAndMatch(benchmark::State& state) {
    OrderBook ob("TEST");
    int64_t id = 1;
    for (auto _ : state) {
        ob.addOrder(id++, true, 100, 1000, "client");
        ob.addOrder(id++, false, 100, 1000, "client");
        ob.triggerMatching();
    }
}
BENCHMARK(BM_AddOrderAndMatch)->Iterations(1000)->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();
