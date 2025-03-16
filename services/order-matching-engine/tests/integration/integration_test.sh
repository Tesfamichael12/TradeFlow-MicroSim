#!/usr/bin/env bash
set -euo pipefail

# integration_test.sh
# Starts the order-matching engine, runs the Python client, asserts expected trade output,
# then stops the server and exits with non-zero on failure.

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
BIN="$ROOT_DIR/build/order-matching-engine"
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

echo "Running Python client..."
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

exit 0
