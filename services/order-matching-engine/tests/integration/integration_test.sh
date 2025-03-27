#!/usr/bin/env bash
set -euo pipefail

# integration_test.sh
# Starts the order-matching engine, runs the Python client, asserts expected trade output,
# then stops the server and exits with non-zero on failure.

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
TESTS_ROOT="$(cd "$ROOT_DIR/.." && pwd)"
PROJECT_ROOT="$(cd "$ROOT_DIR/../.." && pwd)"

if [[ -n "${ORDER_MATCHING_ENGINE_BIN:-}" ]]; then
  BIN="${ORDER_MATCHING_ENGINE_BIN}"
else
  BIN="$PROJECT_ROOT/build/order-matching-engine"
  if [[ ! -x "$BIN" ]]; then
    BIN="$TESTS_ROOT/build/order-matching-engine"
  fi
fi
CLIENT="$ROOT_DIR/test_client.py"
LOGFILE="$ROOT_DIR/server.test.log"
PIDFILE="$ROOT_DIR/server.test.pid"

if [[ ! -x "$BIN" ]]; then
  echo "Binary not found or not executable: $BIN"
  exit 2
fi

rm -f "$LOGFILE" "$PIDFILE"

echo "Starting server..."
nohup "$BIN" > "$LOGFILE" 2>&1 &
echo $! > "$PIDFILE"
sleep 0.5

if ! ss -ltnp | grep -q ":50051"; then
  echo "Server did not start listening on :50051"
  cat "$LOGFILE"
  kill "$(cat $PIDFILE)" || true
  exit 3
fi

if ! python3 -c "import grpc" >/dev/null 2>&1; then
  echo "Installing Python grpc dependency..."
  if ! python3 -m pip --version >/dev/null 2>&1; then
    echo "pip is required to install grpc for Python"
    kill "$(cat $PIDFILE)" || true
    exit 6
  fi
  python3 -m pip install --user --quiet grpcio grpcio-tools
fi

echo "Running Python client..."
PYTHONPATH="$PROJECT_ROOT:$PROJECT_ROOT/build:$TESTS_ROOT:${PYTHONPATH:-}"
export PYTHONPATH
OUTPUT=$(python3 "$CLIENT" || true)
echo "$OUTPUT"

echo "Server log tail:"
tail -n 50 "$LOGFILE"

# Quick assertions: client should report accepted orders and server should have executed a trade
if ! echo "$OUTPUT" | grep -q "ACCEPTED"; then
  echo "Client did not accept orders"
  kill "$(cat $PIDFILE)" || true
  exit 4
fi

if ! tail -n 50 "$LOGFILE" | grep -q "Trade:"; then
  echo "No trade recorded in server log"
  kill "$(cat $PIDFILE)" || true
  exit 5
fi

echo "Integration test passed"

kill "$(cat $PIDFILE)" || true
rm -f "$PIDFILE"

# Ensure benchmark binary is available for downstream CI steps
if [[ ! -x "$PROJECT_ROOT/build/order_bench" ]]; then
  echo "Building benchmark binary..."
  cmake --build "$PROJECT_ROOT/build" --target order_bench -- -j"${CMAKE_BUILD_PARALLEL_LEVEL:-2}"
fi

# Ensure unit test binary exists for valgrind leak checks
if [[ ! -x "$PROJECT_ROOT/build/OrderBook_test" ]]; then
  echo "Building OrderBook_test binary..."
  cmake --build "$PROJECT_ROOT/build" --target OrderBook_test -- -j"${CMAKE_BUILD_PARALLEL_LEVEL:-2}"
fi

# Create benchmark results directory if it doesn't exist
mkdir -p "$PROJECT_ROOT/bench"

exit 0
