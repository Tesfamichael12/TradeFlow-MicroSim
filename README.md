# TradeFlow-MicroSim

Real-Time Stock Trading Simulator

## Overview

This project simulates real-time stock trading with a microservices architecture, including market data handling, high-frequency trading simulation, API gateway, and a web frontend. It uses Redis for caching, Prometheus for metrics, and Grafana for visualization.

## Architecture

- **Market Data Handler** (C++): Handles real-time market data feeds and order matching.
- **HFT Simulator** (C++): Simulates high-frequency trading strategies.
- **API Gateway** (Java): Provides RESTful API endpoints for communication between services and frontend.
- **Frontend** (Next.js): Web interface for users to interact with the simulator.
- **Infrastructure**:
  - Redis: Caching layer.
  - Prometheus: Metrics collection.
  - Grafana: Metrics visualization.

## Getting Started

1. Ensure Docker and Docker Compose are installed.
2. Clone the repository.
3. Run `docker-compose up` from the `deployment/` directory.

## Services

### Market Data Handler

Located in `services/market-data-handler/`

- Built with C++
- Uses CMake for build system

### HFT Simulator

Located in `services/hft-simulator/`

- Built with C++
- Simulates trading algorithms

### API Gateway

Located in `services/api-gateway/`

- Built with Java (Spring Boot assumed)
- Handles API requests

### Frontend

Located in `frontend/`

- Next.js application
- Provides UI for trading simulation

### Infrastructure

Located in `infrastructure/`

- Redis for caching
- Prometheus for monitoring
- Grafana for dashboards

## Development

Each service can be developed independently. Use Docker for containerization.

## License

MIT
