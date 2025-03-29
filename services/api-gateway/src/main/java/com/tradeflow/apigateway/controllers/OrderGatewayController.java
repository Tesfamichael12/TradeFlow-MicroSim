package com.tradeflow.apigateway.controllers;

import com.tradeflow.order.OrderServiceGrpc;
import com.tradeflow.order.OrderServiceOuterClass;
import io.grpc.ManagedChannel;
import io.grpc.ManagedChannelBuilder;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.MediaType;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/v1")
public class OrderGatewayController {

    private final OrderServiceGrpc.OrderServiceBlockingStub blockingStub;

    public OrderGatewayController(@Value("${orderengine.host:localhost}") String host,
                                  @Value("${orderengine.port:50052}") int port) {
        ManagedChannel channel = ManagedChannelBuilder.forAddress(host, port)
                .usePlaintext()
                .build();
        blockingStub = OrderServiceGrpc.newBlockingStub(channel);
    }

    @PostMapping(value = "/orders", consumes = MediaType.APPLICATION_JSON_VALUE, produces = MediaType.APPLICATION_JSON_VALUE)
    public OrderServiceOuterClass.SubmitOrderResponse submitOrder(@RequestBody OrderServiceOuterClass.SubmitOrderRequest req) {
        return blockingStub.submitOrder(req);
    }

    @GetMapping(value = "/orderbook/{symbol}", produces = MediaType.APPLICATION_JSON_VALUE)
    public OrderServiceOuterClass.GetOrderBookResponse getOrderBook(@PathVariable String symbol) {
        OrderServiceOuterClass.GetOrderBookRequest req = OrderServiceOuterClass.GetOrderBookRequest.newBuilder().setSymbol(symbol).build();
        return blockingStub.getOrderBook(req);
    }

    @PostMapping(value = "/orders/{orderId}:cancel", consumes = MediaType.APPLICATION_JSON_VALUE, produces = MediaType.APPLICATION_JSON_VALUE)
    public OrderServiceOuterClass.CancelOrderResponse cancelOrder(@PathVariable("orderId") String orderId, @RequestBody OrderServiceOuterClass.CancelOrderRequest body) {
        OrderServiceOuterClass.CancelOrderRequest req = OrderServiceOuterClass.CancelOrderRequest.newBuilder().setOrderId(orderId).setClientId(body.getClientId()).build();
        return blockingStub.cancelOrder(req);
    }

    @PatchMapping(value = "/orders/{orderId}", consumes = MediaType.APPLICATION_JSON_VALUE, produces = MediaType.APPLICATION_JSON_VALUE)
    public OrderServiceOuterClass.ModifyOrderResponse modifyOrder(@PathVariable("orderId") String orderId, @RequestBody OrderServiceOuterClass.ModifyOrderRequest body) {
        OrderServiceOuterClass.ModifyOrderRequest req = OrderServiceOuterClass.ModifyOrderRequest.newBuilder()
                .setOrderId(orderId)
                .setNewPrice(body.getNewPrice())
                .setNewQuantity(body.getNewQuantity())
                .setClientId(body.getClientId())
                .build();
        return blockingStub.modifyOrder(req);
    }

}
