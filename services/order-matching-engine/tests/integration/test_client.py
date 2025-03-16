#!/usr/bin/env python3
import grpc
import order_service_pb2
import order_service_pb2_grpc

def test_order_matching():
    channel = grpc.insecure_channel('localhost:50051')
    stub = order_service_pb2_grpc.OrderServiceStub(channel)

    # Test submitting orders
    print("Testing Order Matching Engine...")
    print("=" * 50)

    # Submit buy order
    buy_request = order_service_pb2.SubmitOrderRequest(
        symbol="AAPL",
        side="BUY",
        type="LIMIT",
        price=150.00,
        quantity=100,
        client_id="client1"
    )

    try:
        response = stub.SubmitOrder(buy_request)
        print(f"✅ Buy Order Response: {response.status} - {response.message}")
        print(f"   Order ID: {response.order_id}")
    except grpc.RpcError as e:
        print(f"❌ Buy Order Error: {e}")

    # Submit sell order
    sell_request = order_service_pb2.SubmitOrderRequest(
        symbol="AAPL",
        side="SELL",
        type="LIMIT",
        price=150.00,
        quantity=50,
        client_id="client2"
    )

    try:
        response = stub.SubmitOrder(sell_request)
        print(f"✅ Sell Order Response: {response.status} - {response.message}")
        print(f"   Order ID: {response.order_id}")
    except grpc.RpcError as e:
        print(f"❌ Sell Order Error: {e}")

    # Get order book
    book_request = order_service_pb2.GetOrderBookRequest(symbol="AAPL")
    try:
        book_response = stub.GetOrderBook(book_request)
        print(f"\n📊 Order Book Status:")
        print(f"   Bids: {len(book_response.bids)}, Asks: {len(book_response.asks)}")

        if book_response.bids:
            print("   Top 3 Bids:")
            for i, bid in enumerate(book_response.bids[:3]):
                print(f"     {i+1}. {bid.price:.2f} @ {bid.quantity}")

        if book_response.asks:
            print("   Top 3 Asks:")
            for i, ask in enumerate(book_response.asks[:3]):
                print(f"     {i+1}. {ask.price:.2f} @ {ask.quantity}")

    except grpc.RpcError as e:
        print(f"❌ Order Book Error: {e}")

    # Test order cancellation
    if 'response' in locals() and response.status == "ACCEPTED":
        cancel_request = order_service_pb2.CancelOrderRequest(
            order_id=response.order_id,
            client_id="client2"
        )
        try:
            cancel_response = stub.CancelOrder(cancel_request)
            print(f"\n✅ Cancel Order Response: {cancel_response.status} - {cancel_response.message}")
        except grpc.RpcError as e:
            print(f"❌ Cancel Order Error: {e}")

    print("\n" + "=" * 50)
    print("Test completed! Check the service logs for trade executions.")

if __name__ == '__main__':
    test_order_matching()
