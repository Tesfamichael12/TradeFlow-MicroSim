#!/usr/bin/env bash
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/.." && pwd)
BUILD_DIR="$ROOT/services/order-matching-engine/build"

mkdir -p "$BUILD_DIR"
cmake -S "$ROOT/services/order-matching-engine" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR" -j

if [ -x "$BUILD_DIR/order_bench" ]; then
  "$BUILD_DIR/order_bench" --benchmark_repetitions=3 --benchmark_out="$ROOT/services/order-matching-engine/bench/results.json" --benchmark_out_format=json
  echo "Benchmark results written to services/order-matching-engine/bench/results.json"
else
  echo "order_bench binary not found in $BUILD_DIR"
  exit 1
fi
