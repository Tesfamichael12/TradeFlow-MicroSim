# "WarpSpeed" HFT Simulator

> **Performance is a Feature. Latency is the Enemy.**

![C++](https://img.shields.io/badge/C++-17-blue.svg)
![gRPC](https://img.shields.io/badge/gRPC-High%20Performance-green.svg)
![Docker](https://img.shields.io/badge/Docker-Containerized-blueviolet.svg)
![CMake](https://img.shields.io/badge/Build-CMake-orange.svg)

---

## Overview

**WarpSpeed** is a high-performance, low-latency High-Frequency Trading (HFT) simulator developed in modern **C++17**.  
It is designed as a standalone microservice, serving as the **core trade-matching engine** within a distributed financial services application.

- Communication is exclusively through **gRPC** for speed and interoperability.  
- Fully containerized with **Docker** for consistency, portability, and deployment.  
- Demonstrates advanced C++ programming, system design, and performance-critical FinTech engineering.

---

## Core Features

- **High-Performance In-Memory Order Book**: Efficient, non-blocking, price-time priority limit order book.  
- **Asynchronous Matching Engine**: Matches buy/sell orders and executes trades.  
- **gRPC-Based API**: High-throughput external communication for order management & streaming.  
- **Real-Time Market Data Streaming**: Server-side gRPC streams broadcasting live quotes & trades.  
- **Containerized & Portable**: Deployable as an independent microservice.

---

## System Architecture

WarpSpeed is designed as a backend service, receiving commands from an API Gateway or upstream services.  
It specializes in one task: **matching orders at speed**.

```mermaid
graph TD
    A[API Gateway / Other Microservices] -->|gRPC Request| B{WarpSpeed HFT Simulator};
    B -->|gRPC Stream| A;

    subgraph WarpSpeed Service (Docker Container)
        B -- routes --> C[gRPC Service Layer];
        C -- interacts with --> D[Matching Engine];
        D -- manages --> E[Limit Order Book];
        D -- broadcasts events to --> C;
    end
````

* **API Gateway**: Entry point for external requests (REST/WebSocket → gRPC).
* **WarpSpeed HFT Simulator**: Maintains the order book & executes trades.
* **gRPC Stream**: Pushes low-latency market data to clients.

---

## Technology Stack

* **Language**: C++17
* **API/RPC Framework**: gRPC & Protocol Buffers
* **Build System**: CMake
* **Containerization**: Docker

---

## API Endpoint Documentation

All RPCs are defined in `api/warpspeed.proto`.

### 1. `SubmitOrder`

Submit a new limit order to the matching engine.

**Definition:**

```proto
rpc SubmitOrder(OrderRequest) returns (OrderResponse)
```

**Request:**

```json
{
  "order": {
    "order_id": "BUY_ORDER_1",
    "instrument": "BTC/USD",
    "price": 101.50,
    "quantity": 10,
    "side": "BUY"
  }
}
```

**Response:**

```json
{
  "status": "SUCCESS",
  "order_id": "BUY_ORDER_1"
}
```

---

### 2. `CancelOrder`

Cancel an active limit order not yet fully filled.

**Definition:**

```proto
rpc CancelOrder(CancelRequest) returns (CancelResponse)
```

**Request:**

```json
{
  "order_id": "CANCEL_ME_1"
}
```

**Response:**

```json
{
  "status": "SUCCESS: Order cancelled."
}
```

---

### 3. `StreamMarketData`

Subscribe to a continuous stream of market data for a given instrument.

**Definition:**

```proto
rpc StreamMarketData(MarketDataRequest) returns (stream MarketData)
```

**Request:**

```json
{
  "instrument": "BTC/USD"
}
```

**Response Examples:**

* **Quote Update:**

```json
{
  "instrument": "BTC/USD",
  "timestamp": 1677611383123,
  "quote": {
    "bid_price": 101.49,
    "ask_price": 101.50,
    "bid_quantity": 50,
    "ask_quantity": 10
  }
}
```

* **Trade Execution:**

```json
{
  "instrument": "BTC/USD",
  "timestamp": 1677611385456,
  "trade": {
    "price": 101.50,
    "quantity": 10
  }
}
```

---

## Testing the Service

This project includes a **gRPC test client (`grpc_client_test`)** to validate endpoints.
Run with two terminals:

1. **Build the project** (see [Build Instructions](#build-and-run-instructions)).
2. **Start the server** (Terminal 1):

```bash
# Windows
.\build\Debug\warpspeed_server.exe

# Linux/macOS
./build/warpspeed_server
```

3. **Run the client** (Terminal 2):

```bash
# Windows
.\build\Debug\grpc_client_test.exe

# Linux/macOS
./build/grpc_client_test
```

4. **Observe Output**: Orders submitted, trades confirmed, cancellations verified.
   Successful runs end with:

```
VERIFICATION SUCCESS
Test Client Finished
```

---

## Build and Run Instructions

### Prerequisites

* C++17 compliant compiler
* CMake (3.15+)
* gRPC & Protocol Buffers installed

### Local Build

```bash
# Clone repo
git clone <your-repo-url>
cd warpspeed_hft_simulator

# Configure
cmake -B build

# Compile
cmake --build build
```

Run server and client as shown in [Testing](#testing-the-service).

---

### Docker Build (Recommended)

```bash
# Build image
docker build -t warpspeed-hft -f docker/Dockerfile .

# Run container
docker run -p 50052:50052 --name hft-simulator warpspeed-hft
```

The server will be running on **localhost:50052**.
