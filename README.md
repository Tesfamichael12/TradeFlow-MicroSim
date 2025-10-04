# TradeFlow-MicroSim

Real-Time Stock Trading Simulator on a polyglot microservices stack.

## Overview

TradeFlow-MicroSim models an exchange workflow end to end:

- **Order ingestion & matching** via a high-performance C++20 gRPC service.
- **High-frequency strategy simulation** with the WarpSpeed HFT engine.
- **HTTP access** through a Java Spring Boot API gateway that bridges to gRPC.
- **Reactive UI** built with Next.js for operators and traders.
- **Operational plumbing** (Redis cache, Prometheus, Grafana) to round out the deployment footprint.

This repository focuses on the microservice-oriented implementation. A legacy single-binary prototype continues to live in `../Real-Time-Market-Data-Feed-Handler-and-Order-Matching-Engine/` for reference.

## Component map

| Component               | Language             | Path                              | Purpose                                                                 | Exposed port |
| ----------------------- | -------------------- | --------------------------------- | ----------------------------------------------------------------------- | ------------ |
| Order Matching Engine   | C++20, gRPC          | `services/order-matching-engine/` | Price/time priority order book with streaming updates                   | `50051`      |
| WarpSpeed HFT Simulator | C++17, gRPC          | `services/hft-simulator/`         | Generates high-volume order/order-cancel flow and streams quotes/trades | `50052`      |
| Market Data Handler     | C++                  | `services/market-data-handler/`   | Planned market data feed normalizer (currently scaffold)                | TBC          |
| API Gateway             | Java 17, Spring Boot | `services/api-gateway/`           | REST façade that calls the C++ gRPC backend via generated stubs         | `8080`       |
| Frontend                | Next.js 12, React 17 | `frontend/`                       | Dashboard and control panel for the simulator                           | `3000`       |
| Redis                   | Docker               | `infrastructure/redis/`           | Shared cache and pub/sub backbone                                       | `6379`       |
| Prometheus              | Docker               | `infrastructure/prometheus/`      | Metrics scraper (order-matching + gateway exporters wired)              | `9090`       |
| Grafana                 | Docker               | `infrastructure/grafana/`         | Dashboards for simulator metrics                                        | `3001`       |

Related assets:

- `deployment/docker-compose.yml` ties the services together. The compose file currently expects an `hft-simulator` service definition; add it (using `services/hft-simulator/Dockerfile`) or start the binary manually before bringing up the dependent services.
- `services/order-matching-engine/proto/order_service.proto` is the canonical contract used across languages.
- Python gRPC test clients live under `services/order-matching-engine/tests/integration/`.

## Quick start (containerised)

1. Install Docker and Docker Compose v2.
1. (Optional but recommended) Build the HFT simulator image once:

```sh
docker build -f ../services/hft-simulator/Dockerfile -t tradeflow-hft-simulator ../services/hft-simulator
```

1. Define the `hft-simulator` service in `deployment/docker-compose.yml` (see notes above) or run the binary locally.
1. From `deployment/`, launch the stack:

```sh
docker compose up --build
```

Endpoints to check once the stack is running:

- API Gateway: <http://localhost:8080>
- Frontend UI: <http://localhost:3000>
- Prometheus UI: <http://localhost:9090>
- Grafana: <http://localhost:3001> (default admin password `admin`).
- Order Matching metrics endpoint: <http://localhost:9464/metrics> (text exposition).

When developing locally without containers, read the service-specific sections below for build/run steps.

## Service deep dive

### Order Matching Engine (C++20)

- Implements multi-symbol limit order books with price–time priority matching, trade logging, and gRPC streaming endpoints defined in `order_service.proto`.
- Build & run:

  ```sh
  mkdir -p services/order-matching-engine/build
  cmake -S services/order-matching-engine -B services/order-matching-engine/build -DCMAKE_BUILD_TYPE=Release
  cmake --build services/order-matching-engine/build -j
  ./services/order-matching-engine/build/order-matching-engine
  ```

- Unit tests & integration tests live under `services/order-matching-engine/tests/`. Use `ctest --output-on-failure` after configuring the build tree.
- Memory-leak regression suite:

  ```sh
  ./scripts/memory_test.sh
  cat services/order-matching-engine/out/memory_test_summary.md
  ```

  The harness runs unit, stress, and gRPC integration workloads under Valgrind and fails CI if _any_ bytes remain in use at exit.

- Micro-benchmarks:

  ```sh
  ./scripts/run_bench.sh
  ```

  Results are emitted as JSON to `services/order-matching-engine/bench/results.json` and can be parsed with `scripts/parse_bench.py` for summaries.

- Generated client stubs for Python (`order_service_pb2*.py`) are kept for integration tests; Java stubs are generated on the fly by the gateway build.

#### Benchmark summary (Order Matching Engine)

| Metric  |       Value |
| ------- | ----------: |
| p50     | 0.512468 µs |
| p90     | 0.571549 µs |
| p99     | 0.593002 µs |
| p99.9   | 0.595148 µs |
| mean    | 0.523965 µs |
| samples |          10 |

Context:

- host: `pop-os` Linux
- date: `2025-03-25T08:21:45+03:00`
- cpus: `8`

### WarpSpeed HFT Simulator (C++17)

- Provides a gRPC surface (`warpspeed.proto`) to submit/cancel orders and subscribe to market data from strategy bots.
- `services/hft-simulator/src/grpc/server.cpp` hosts the server, backed by the asynchronous matching engine in `src/core/`.
- Build & run:

  ```sh
  cmake -S services/hft-simulator -B services/hft-simulator/build
  cmake --build services/hft-simulator/build -j
  ./services/hft-simulator/build/warpspeed_server
  ```

- A lightweight gRPC client harness is included in the build tree (`grpc_client_test`) for smoke tests.
- Docker image scaffolding is available at `services/hft-simulator/Dockerfile` for container deployment.

### Market Data Handler (C++)

- Currently a skeleton awaiting implementation. Its `main.cpp` is a placeholder and should be replaced with a real feed adapter that normalises ticks for downstream services.
- Build tooling (CMake, Dockerfile) is in place so future work can focus solely on business logic.

### API Gateway (Java Spring Boot)

- Exposes REST/JSON endpoints and proxies calls to the C++ matching engine over gRPC by using generated Java stubs.
- `pom.xml` wires the `protobuf-maven-plugin`; running `mvn clean compile` regenerates stubs from the shared proto files and keeps them under `target/generated-sources/` (ignored by Git).
- Basic usage:

  ```sh
  cd services/api-gateway
  mvn clean package
  java -jar target/api-gateway-1.0.0.jar --orderengine.host=localhost --orderengine.port=50051
  ```

  The gateway exposes Prometheus metrics at <http://localhost:8080/actuator/prometheus> when running locally. Omit the `--orderengine.*` overrides when the service runs inside Docker Compose (defaults point to `order-matching-engine:50051`).

### Frontend (Next.js)

- Provides the simulator dashboard.
- Dev workflow:

  ```sh
  cd frontend
  npm install
  npm run dev
  ```

- Production build: `npm run build && npm run start` (uses port 3000 by default).

## Testing & quality gates

- **Unit & integration tests**: Driven by CTest for the C++ services and JUnit (to be added) for the Java gateway.
- **Memory regression**: `scripts/memory_test.sh` runs Valgrind against unit, stress, and integration workloads. Latest run (2025‑03‑25 on Ubuntu 22.04, 8c/32 GB) produced:

  | Test Type        | Status  | Memory leaks | Peak RSS |
  | ---------------- | ------- | ------------ | -------- |
  | Unit Tests       | ✅ PASS | 0 bytes      | 2.1 MB   |
  | Stress Test      | ✅ PASS | 0 bytes      | 156.7 MB |
  | Integration Test | ✅ PASS | 0 bytes      | 89.2 MB  |

  Full logs live under `services/order-matching-engine/out/`.

- **Benchmarks**: `scripts/run_bench.sh` captures latency distributions for core matching paths. Use `scripts/parse_bench.py` to transform the JSON output into human-readable summaries.
- **Static analysis**: Not yet automated. Clang-Tidy and cpplint hooks are recommended additions.

## Observability stack

- Prometheus (`infrastructure/prometheus/`) scrapes:
  - `order-matching-engine:9464/metrics` – lightweight exporter baked into the C++ gRPC server with counters for RPC traffic, cancellations, trades, and streaming subscribers.
  - `api-gateway:8080/actuator/prometheus` – Spring Boot Actuator + Micrometer exposes request timers and success/error counters for the REST façade.
- Grafana (`infrastructure/grafana/`) is pre-provisioned with a Prometheus data source and a starter _Order Matching Engine Overview_ dashboard (`TradeFlow` folder) showing submit volumes, trade throughput, cancel outcomes, and gateway latency trends.
- Redis (`infrastructure/redis/`) provides shared state. Docker Compose exposes the default port (`6379`) for local debugging.

### Upcoming work

- Add native exporters for the WarpSpeed HFT simulator and the market data handler once their business logic firms up.
- Expand Grafana dashboards with order book depth visualisations and alerting once additional metrics land.

## Development tips

- Regenerate gRPC stubs for new proto changes before compiling dependants:
  - `cmake --build services/order-matching-engine/build --target order_service_proto` (C++).
  - `mvn generate-sources` (Java gateway).
  - `python -m grpc_tools.protoc` (if Python clients need updating).
- Keep generated artifacts out of Git: `.gitignore` already ignores Maven’s `target/` and the Java stub directory.
- Use the Valgrind and benchmark scripts early when touching matching-engine internals—performance and leak regressions surface quickly under load.

## License

MIT
