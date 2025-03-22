FROM ubuntu:22.04 as builder
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libgrpc++-dev \
    libgrpc-dev \
    libprotobuf-dev \
    protobuf-compiler-grpc \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /build
COPY . .
RUN mkdir -p build && cd build \
    && cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_FLAGS="-O3 -march=native" \
    && cmake --build . --config Release -j$(nproc)
FROM ubuntu:22.04-slim
RUN apt-get update && apt-get install -y \
    libgrpc++1 \
    libprotobuf23 \
    && rm -rf /var/lib/apt/lists/*
RUN useradd -m -s /bin/bash warpspeed
WORKDIR /app
COPY --from=builder /build/build/warpspeed_server .
RUN chown -R warpspeed:warpspeed /app
USER warpspeed

EXPOSE 50052

ENV GRPC_POLL_STRATEGY=epoll
ENV GRPC_ENABLE_FORK_SUPPORT=0

ENTRYPOINT ["/app/warpspeed_server"]