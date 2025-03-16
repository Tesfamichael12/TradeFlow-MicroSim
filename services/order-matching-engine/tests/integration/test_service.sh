#!/bin/bash

echo "Testing Order Matching Engine Service"
echo "===================================="

# Check if service is running
if ! nc -z localhost 50051 2>/dev/null; then
    echo "❌ Service is not running on port 50051"
    echo "Please start the service first:"
    echo "  docker run -p 50051:50051 order-matching-engine"
    echo "  OR"
    echo "  ./order-matching-engine (if built locally)"
    exit 1
fi

echo "✅ Service is running on port 50051"

# Test with curl (if grpcurl is available)
if command -v grpcurl &> /dev/null; then
    echo "Testing with grpcurl..."
    
    # Test order submission
    echo "Submitting buy order..."
    grpcurl -plaintext -d '{"symbol":"AAPL","side":"BUY","type":"LIMIT","price":150.0,"quantity":100,"clientId":"test1"}' \
        localhost:50051 tradeflow.order.OrderService.SubmitOrder
    
    echo "Submitting sell order..."
    grpcurl -plaintext -d '{"symbol":"AAPL","side":"SELL","type":"LIMIT","price":150.0,"quantity":50,"clientId":"test2"}' \
        localhost:50051 tradeflow.order.OrderService.SubmitOrder
    
    echo "Getting order book..."
    grpcurl -plaintext -d '{"symbol":"AAPL"}' \
        localhost:50051 tradeflow.order.OrderService.GetOrderBook
else
    echo "grpcurl not found. Install it with:"
    echo "  go install github.com/fullstorydev/grpcurl@latest"
    echo ""
    echo "Or use the Python test client:"
    echo "  python3 test_client.py"
fi

echo ""
echo "Test completed!"
