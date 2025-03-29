FROM maven:3.8.8-jdk-11 AS builder

# Copy repo into the builder image. We copy the full repo so the protobuf plugin
# can access sibling proto files at ../order-matching-engine/proto when building.
WORKDIR /workspace
COPY . .

# Build the api-gateway module (skip tests to speed up builds in CI/dev)
RUN mvn -f services/api-gateway/pom.xml -pl services/api-gateway -am clean package -DskipTests

FROM openjdk:11-jre-slim

WORKDIR /app

# Environment variable users can override at runtime to point the gateway at the engine
ENV ORDERENGINE_URL=http://host.docker.internal:8080

# Copy the built jar from the builder stage
COPY --from=builder /workspace/services/api-gateway/target/*.jar ./api-gateway.jar

EXPOSE 8080

ENTRYPOINT ["sh","-c","exec java -jar /app/api-gateway.jar"]
