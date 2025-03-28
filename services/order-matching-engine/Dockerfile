# Multi-stage Dockerfile
# Builder: build the C++ server with CMake and protoc
FROM ubuntu:22.04 AS builder
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git wget ca-certificates pkg-config \
    libgrpc++-dev libprotobuf-dev protobuf-compiler protobuf-compiler-grpc \
    libssl-dev curl && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /work
COPY . /work

# Build
RUN mkdir -p build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . --target order-matching-engine -j$(nproc) || \
    cmake --build . -j$(nproc)

# Runtime: minimal image with the compiled binary
FROM ubuntu:22.04 AS runtime
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates libstdc++6 libgcc1 && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /work/build/order-matching-engine /app/order-matching-engine

EXPOSE 50051 9464
ENTRYPOINT ["/app/order-matching-engine"]
FROM ubuntu:22.04

WORKDIR /app

COPY . .

# Install build tools, protobuf and gRPC dev packages
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    git \
    libprotobuf-dev \
    protobuf-compiler \
    libgrpc++-dev \
    libgrpc-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Build the project
RUN cmake . && make -j$(nproc)

EXPOSE 50051 9464

CMD ["/app/order-matching-engine"]
