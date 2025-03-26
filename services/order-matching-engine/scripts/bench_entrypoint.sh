#!/usr/bin/env bash
set -euo pipefail

WORKDIR=/workspace
OUTDIR=/workspace/out
BUILD_DIR=$WORKDIR/build

mkdir -p "$OUTDIR"
rm -rf "$BUILD_DIR"
cmake -S "$WORKDIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" -j"$(nproc)"

# Run benchmarks if available
if [ -x "$BUILD_DIR/order_bench" ]; then
  echo "Running benchmark..."
  "$BUILD_DIR/order_bench" --benchmark_repetitions=3 --benchmark_out="$OUTDIR/bench_results.json" --benchmark_out_format=json
else
  echo "order_bench not built. Skipping benchmarks."
fi

# Run replay runner if available
if [ -x "$BUILD_DIR/replay_runner" ]; then
  echo "Running replay runner..."
  "$BUILD_DIR/replay_runner" /workspace/testdata/replays/simple_replay.json > "$OUTDIR/replay_output.txt" 2>&1 || true
else
  echo "replay_runner not built. Skipping replay."
fi

# Copy build artifacts
cp -r "$BUILD_DIR"/order-matching-engine "$OUTDIR/" || true

# Keep container alive briefly so files are synced
sleep 1

# Print outputs summary
echo "Outputs in $OUTDIR:"
ls -la "$OUTDIR" || true

