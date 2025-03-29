# API Gateway (Java Spring Boot) — setup

This gateway provides HTTP/JSON endpoints that forward requests to the Order Matching Engine gRPC service.

Prerequisites

- JDK 17+
- Maven
- The generated Java gRPC stubs for `order_service.proto` should be available on the classpath (either compiled in the project or installed as a dependency). You can generate them with the `protoc` + `protoc-gen-grpc-java` plugin and add to your Maven build.

Run the gateway

1. Build the gateway

```bash
cd services/api-gateway
mvn package
```

1. Start the gateway (pointing to the order matching engine)

```bash
# default: orderengine.host=localhost, orderengine.port=50051
java -jar target/api-gateway-0.0.1-SNAPSHOT.jar --orderengine.host=localhost --orderengine.port=50051
```

Notes

- The gateway uses the gRPC blocking stub for unary calls. For streaming (SubscribeTrades) you'll need to add an async controller and a websocket/SSE adapter.
