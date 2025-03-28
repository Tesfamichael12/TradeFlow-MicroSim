package com.tradeflow.apigateway.controllers;

import com.tradeflow.order.OrderServiceGrpc;
import com.tradeflow.order.OrderServiceOuterClass;
import io.grpc.ManagedChannel;
import io.grpc.ManagedChannelBuilder;
import io.micrometer.core.instrument.Counter;
import io.micrometer.core.instrument.MeterRegistry;
import io.micrometer.core.instrument.Timer;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.MediaType;
import org.springframework.web.bind.annotation.*;

import javax.annotation.PreDestroy;

@RestController
@RequestMapping("/v1")
public class OrderGatewayController {

    private final ManagedChannel channel;
    private final OrderServiceGrpc.OrderServiceBlockingStub blockingStub;
    private final MeterRegistry meterRegistry;
    private final Counter submitOrderRequests;
    private final Counter submitOrderAccepted;
    private final Counter submitOrderRejected;
    private final Counter cancelOrderRequests;
    private final Counter cancelOrderErrors;
    private final Counter modifyOrderRequests;
    private final Counter modifyOrderErrors;
    private final Timer submitOrderTimer;
    private final Timer getOrderBookTimer;
    private final Timer cancelOrderTimer;
    private final Timer modifyOrderTimer;

    public OrderGatewayController(@Value("${orderengine.host:localhost}") String host,
                                  @Value("${orderengine.port:50051}") int port,
                                  MeterRegistry meterRegistry) {
        this.channel = ManagedChannelBuilder.forAddress(host, port)
                .usePlaintext()
                .build();
        this.blockingStub = OrderServiceGrpc.newBlockingStub(channel);
        this.meterRegistry = meterRegistry;

        this.submitOrderRequests = meterRegistry.counter("tradeflow_gateway_submit_order_requests_total");
        this.submitOrderAccepted = meterRegistry.counter("tradeflow_gateway_submit_order_accepted_total");
        this.submitOrderRejected = meterRegistry.counter("tradeflow_gateway_submit_order_rejected_total");
        this.cancelOrderRequests = meterRegistry.counter("tradeflow_gateway_cancel_order_requests_total");
        this.cancelOrderErrors = meterRegistry.counter("tradeflow_gateway_cancel_order_errors_total");
        this.modifyOrderRequests = meterRegistry.counter("tradeflow_gateway_modify_order_requests_total");
        this.modifyOrderErrors = meterRegistry.counter("tradeflow_gateway_modify_order_errors_total");

        this.submitOrderTimer = meterRegistry.timer("tradeflow_gateway_submit_order_duration_seconds");
        this.getOrderBookTimer = meterRegistry.timer("tradeflow_gateway_get_orderbook_duration_seconds");
        this.cancelOrderTimer = meterRegistry.timer("tradeflow_gateway_cancel_order_duration_seconds");
        this.modifyOrderTimer = meterRegistry.timer("tradeflow_gateway_modify_order_duration_seconds");
    }

    @PreDestroy
    public void shutdown() {
        if (channel != null) {
            channel.shutdown();
        }
    }

    @PostMapping(value = "/orders", consumes = MediaType.APPLICATION_JSON_VALUE, produces = MediaType.APPLICATION_JSON_VALUE)
    public OrderServiceOuterClass.SubmitOrderResponse submitOrder(@RequestBody OrderServiceOuterClass.SubmitOrderRequest req) {
        submitOrderRequests.increment();
        Timer.Sample sample = Timer.start(meterRegistry);
        OrderServiceOuterClass.SubmitOrderResponse response = blockingStub.submitOrder(req);
        sample.stop(submitOrderTimer);
        if ("ACCEPTED".equalsIgnoreCase(response.getStatus())) {
            submitOrderAccepted.increment();
        } else {
            submitOrderRejected.increment();
        }
        return response;
    }

    @GetMapping(value = "/orderbook/{symbol}", produces = MediaType.APPLICATION_JSON_VALUE)
    public OrderServiceOuterClass.GetOrderBookResponse getOrderBook(@PathVariable String symbol) {
        Timer.Sample sample = Timer.start(meterRegistry);
        OrderServiceOuterClass.GetOrderBookRequest req = OrderServiceOuterClass.GetOrderBookRequest.newBuilder().setSymbol(symbol).build();
        OrderServiceOuterClass.GetOrderBookResponse response = blockingStub.getOrderBook(req);
        sample.stop(getOrderBookTimer);
        return response;
    }

    @PostMapping(value = "/orders/{orderId}:cancel", consumes = MediaType.APPLICATION_JSON_VALUE, produces = MediaType.APPLICATION_JSON_VALUE)
    public OrderServiceOuterClass.CancelOrderResponse cancelOrder(@PathVariable("orderId") String orderId, @RequestBody OrderServiceOuterClass.CancelOrderRequest body) {
        cancelOrderRequests.increment();
        Timer.Sample sample = Timer.start(meterRegistry);
        OrderServiceOuterClass.CancelOrderRequest req = OrderServiceOuterClass.CancelOrderRequest.newBuilder().setOrderId(orderId).setClientId(body.getClientId()).build();
        OrderServiceOuterClass.CancelOrderResponse response = blockingStub.cancelOrder(req);
        sample.stop(cancelOrderTimer);
        if (!"CANCELLED".equalsIgnoreCase(response.getStatus())) {
            cancelOrderErrors.increment();
        }
        return response;
    }

    @PatchMapping(value = "/orders/{orderId}", consumes = MediaType.APPLICATION_JSON_VALUE, produces = MediaType.APPLICATION_JSON_VALUE)
    public OrderServiceOuterClass.ModifyOrderResponse modifyOrder(@PathVariable("orderId") String orderId, @RequestBody OrderServiceOuterClass.ModifyOrderRequest body) {
        modifyOrderRequests.increment();
        Timer.Sample sample = Timer.start(meterRegistry);
        OrderServiceOuterClass.ModifyOrderRequest req = OrderServiceOuterClass.ModifyOrderRequest.newBuilder()
                .setOrderId(orderId)
                .setNewPrice(body.getNewPrice())
                .setNewQuantity(body.getNewQuantity())
                .setClientId(body.getClientId())
                .build();
        OrderServiceOuterClass.ModifyOrderResponse response = blockingStub.modifyOrder(req);
        sample.stop(modifyOrderTimer);
        if (!"MODIFIED".equalsIgnoreCase(response.getStatus())) {
            modifyOrderErrors.increment();
        }
        return response;
    }

}
