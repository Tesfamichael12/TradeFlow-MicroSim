#!/usr/bin/env bash
set -euo pipefail

# Memory leak testing script for TradeFlow MicroSim
# Tests the order-matching engine for memory leaks under stress

ROOT=$(cd "$(dirname "$0")/.." && pwd)
BUILD_DIR="$ROOT/services/order-matching-engine/build"
OUTPUT_DIR="$ROOT/services/order-matching-engine/out"

echo "🔍 Starting memory leak testing for TradeFlow MicroSim..."

# Create output directory
mkdir -p "$OUTPUT_DIR"

# Build with debug symbols for better valgrind output
echo "Building with debug symbols..."
mkdir -p "$BUILD_DIR"
cmake -S "$ROOT/services/order-matching-engine" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Debug
cmake --build "$BUILD_DIR" -j

# Test 1: Unit tests with valgrind
echo "Running unit tests with valgrind..."
valgrind \
  --tool=memcheck \
  --leak-check=full \
  --show-leak-kinds=all \
  --track-origins=yes \
  --verbose \
  --log-file="$OUTPUT_DIR/valgrind_unit_tests.txt" \
  ./"$BUILD_DIR/OrderBook_test"

# Test 2: Stress test with high order volume
echo "Running stress test with high order volume..."
cat > "$OUTPUT_DIR/stress_test.cpp" << 'EOF'
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include "order_matching/OrderBook.hpp"

using namespace tradeflow;

int main() {
    OrderBook ob("STRESS_TEST", MatchingMode::PRICE_TIME_PRIORITY);
    
    // Add many orders to stress test memory management
    const int NUM_ORDERS = 10000;
    const int NUM_THREADS = 4;
    
    std::vector<std::thread> threads;
    
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([&ob, t, NUM_ORDERS]() {
            for (int i = 0; i < NUM_ORDERS / NUM_THREADS; ++i) {
                int64_t order_id = t * (NUM_ORDERS / NUM_THREADS) + i + 1;
                bool is_buy = (i % 2 == 0);
                int32_t quantity = 100 + (i % 1000);
                int64_t price = 10000 + (i % 1000); // Price in ticks
                
                ob.addOrder(order_id, is_buy, quantity, price, "stress_client_" + std::to_string(t));
                
                // Trigger matching every 100 orders
                if (i % 100 == 0) {
                    ob.triggerMatching();
                }
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Final matching
    ob.triggerMatching();
    
    std::cout << "Stress test completed with " << NUM_ORDERS << " orders" << std::endl;
    return 0;
}
EOF

# Compile stress test
g++ -std=c++20 -I"$ROOT/services/order-matching-engine/include" \
    -I"$BUILD_DIR" \
    "$OUTPUT_DIR/stress_test.cpp" \
    "$ROOT/services/order-matching-engine/src/order_matching/OrderBook.cpp" \
    "$ROOT/services/order-matching-engine/src/order_matching/Order.cpp" \
    -o "$OUTPUT_DIR/stress_test"

# Run stress test with valgrind
valgrind \
  --tool=memcheck \
  --leak-check=full \
  --show-leak-kinds=all \
  --track-origins=yes \
  --verbose \
  --log-file="$OUTPUT_DIR/valgrind_stress_test.txt" \
  ./"$OUTPUT_DIR/stress_test"

# Test 3: Integration test with gRPC server
echo "Running integration test with gRPC server..."
# Start the server in background
./"$BUILD_DIR/order-matching-engine" &
SERVER_PID=$!

# Wait for server to start
sleep 2

# Run integration test with valgrind
valgrind \
  --tool=memcheck \
  --leak-check=full \
  --show-leak-kinds=all \
  --track-origins=yes \
  --verbose \
  --log-file="$OUTPUT_DIR/valgrind_integration_test.txt" \
  bash "$ROOT/services/order-matching-engine/tests/integration/integration_test.sh"

# Stop the server
kill $SERVER_PID 2>/dev/null || true

# Analyze results
echo "Analyzing memory leak test results..."

check_memory_leaks() {
    local log_file="$1"
    local test_name="$2"
    
    echo "Checking $test_name..."
    
    if [ ! -f "$log_file" ]; then
        echo "$test_name: Log file not found"
        return 1
    fi
    
    # Check for definitely lost bytes
    local definitely_lost=$(grep "definitely lost:" "$log_file" | awk '{print $4}' | head -1)
    local indirectly_lost=$(grep "indirectly lost:" "$log_file" | awk '{print $4}' | head -1)
    local possibly_lost=$(grep "possibly lost:" "$log_file" | awk '{print $4}' | head -1)
    
    if [ "$definitely_lost" = "0" ] && [ "$indirectly_lost" = "0" ] && [ "$possibly_lost" = "0" ]; then
        echo "$test_name: No memory leaks detected"
        return 0
    else
        echo "$test_name: Memory leaks detected:"
        echo "   Definitely lost: $definitely_lost bytes"
        echo "   Indirectly lost: $indirectly_lost bytes"
        echo "   Possibly lost: $possibly_lost bytes"
        return 1
    fi
}

# Check all test results
FAILED_TESTS=0

check_memory_leaks "$OUTPUT_DIR/valgrind_unit_tests.txt" "Unit Tests" || FAILED_TESTS=$((FAILED_TESTS + 1))
check_memory_leaks "$OUTPUT_DIR/valgrind_stress_test.txt" "Stress Test" || FAILED_TESTS=$((FAILED_TESTS + 1))
check_memory_leaks "$OUTPUT_DIR/valgrind_integration_test.txt" "Integration Test" || FAILED_TESTS=$((FAILED_TESTS + 1))

# Generate summary report
cat > "$OUTPUT_DIR/memory_test_summary.md" << EOF
# Memory Leak Test Summary

## Test Results

| Test Type | Status | Details |
|-----------|--------|---------|
| Unit Tests | $(if check_memory_leaks "$OUTPUT_DIR/valgrind_unit_tests.txt" "Unit Tests" >/dev/null 2>&1; then echo "PASS"; else echo " FAIL"; fi) | Basic order book operations |
| Stress Test | $(if check_memory_leaks "$OUTPUT_DIR/valgrind_stress_test.txt" "Stress Test" >/dev/null 2>&1; then echo "PASS"; else echo "FAIL"; fi) | High volume order processing |
| Integration Test | $(if check_memory_leaks "$OUTPUT_DIR/valgrind_integration_test.txt" "Integration Test" >/dev/null 2>&1; then echo "PASS"; else echo "FAIL"; fi) | gRPC server operations |

## Test Configuration

- **Tool**: Valgrind memcheck
- **Leak Check**: Full
- **Track Origins**: Yes
- **Verbose**: Yes

## Files Generated

- \`valgrind_unit_tests.txt\` - Unit test memory analysis
- \`valgrind_stress_test.txt\` - Stress test memory analysis  
- \`valgrind_integration_test.txt\` - Integration test memory analysis
- \`memory_test_summary.md\` - This summary report

## Conclusion

$(if [ $FAILED_TESTS -eq 0 ]; then echo "All memory leak tests passed. Zero memory leaks detected under stress."; else echo "$FAILED_TESTS test(s) failed. Memory leaks detected."; fi)
EOF

echo "Memory test summary written to $OUTPUT_DIR/memory_test_summary.md"

if [ $FAILED_TESTS -eq 0 ]; then
    echo "All memory leak tests passed! Zero leaks detected under stress."
    exit 0
else
    echo "$FAILED_TESTS test(s) failed. Check the log files for details."
    exit 1
fi
