package com.tradeflow.order;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.56.0)",
    comments = "Source: order_service.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class OrderServiceGrpc {

  private OrderServiceGrpc() {}

  public static final String SERVICE_NAME = "tradeflow.order.OrderService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.tradeflow.order.SubmitOrderRequest,
      com.tradeflow.order.SubmitOrderResponse> getSubmitOrderMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SubmitOrder",
      requestType = com.tradeflow.order.SubmitOrderRequest.class,
      responseType = com.tradeflow.order.SubmitOrderResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.tradeflow.order.SubmitOrderRequest,
      com.tradeflow.order.SubmitOrderResponse> getSubmitOrderMethod() {
    io.grpc.MethodDescriptor<com.tradeflow.order.SubmitOrderRequest, com.tradeflow.order.SubmitOrderResponse> getSubmitOrderMethod;
    if ((getSubmitOrderMethod = OrderServiceGrpc.getSubmitOrderMethod) == null) {
      synchronized (OrderServiceGrpc.class) {
        if ((getSubmitOrderMethod = OrderServiceGrpc.getSubmitOrderMethod) == null) {
          OrderServiceGrpc.getSubmitOrderMethod = getSubmitOrderMethod =
              io.grpc.MethodDescriptor.<com.tradeflow.order.SubmitOrderRequest, com.tradeflow.order.SubmitOrderResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SubmitOrder"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  com.tradeflow.order.SubmitOrderRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  com.tradeflow.order.SubmitOrderResponse.getDefaultInstance()))
              .setSchemaDescriptor(new OrderServiceMethodDescriptorSupplier("SubmitOrder"))
              .build();
        }
      }
    }
    return getSubmitOrderMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.tradeflow.order.GetOrderBookRequest,
      com.tradeflow.order.GetOrderBookResponse> getGetOrderBookMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetOrderBook",
      requestType = com.tradeflow.order.GetOrderBookRequest.class,
      responseType = com.tradeflow.order.GetOrderBookResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.tradeflow.order.GetOrderBookRequest,
      com.tradeflow.order.GetOrderBookResponse> getGetOrderBookMethod() {
    io.grpc.MethodDescriptor<com.tradeflow.order.GetOrderBookRequest, com.tradeflow.order.GetOrderBookResponse> getGetOrderBookMethod;
    if ((getGetOrderBookMethod = OrderServiceGrpc.getGetOrderBookMethod) == null) {
      synchronized (OrderServiceGrpc.class) {
        if ((getGetOrderBookMethod = OrderServiceGrpc.getGetOrderBookMethod) == null) {
          OrderServiceGrpc.getGetOrderBookMethod = getGetOrderBookMethod =
              io.grpc.MethodDescriptor.<com.tradeflow.order.GetOrderBookRequest, com.tradeflow.order.GetOrderBookResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetOrderBook"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  com.tradeflow.order.GetOrderBookRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  com.tradeflow.order.GetOrderBookResponse.getDefaultInstance()))
              .setSchemaDescriptor(new OrderServiceMethodDescriptorSupplier("GetOrderBook"))
              .build();
        }
      }
    }
    return getGetOrderBookMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.tradeflow.order.CancelOrderRequest,
      com.tradeflow.order.CancelOrderResponse> getCancelOrderMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "CancelOrder",
      requestType = com.tradeflow.order.CancelOrderRequest.class,
      responseType = com.tradeflow.order.CancelOrderResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.tradeflow.order.CancelOrderRequest,
      com.tradeflow.order.CancelOrderResponse> getCancelOrderMethod() {
    io.grpc.MethodDescriptor<com.tradeflow.order.CancelOrderRequest, com.tradeflow.order.CancelOrderResponse> getCancelOrderMethod;
    if ((getCancelOrderMethod = OrderServiceGrpc.getCancelOrderMethod) == null) {
      synchronized (OrderServiceGrpc.class) {
        if ((getCancelOrderMethod = OrderServiceGrpc.getCancelOrderMethod) == null) {
          OrderServiceGrpc.getCancelOrderMethod = getCancelOrderMethod =
              io.grpc.MethodDescriptor.<com.tradeflow.order.CancelOrderRequest, com.tradeflow.order.CancelOrderResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "CancelOrder"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  com.tradeflow.order.CancelOrderRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  com.tradeflow.order.CancelOrderResponse.getDefaultInstance()))
              .setSchemaDescriptor(new OrderServiceMethodDescriptorSupplier("CancelOrder"))
              .build();
        }
      }
    }
    return getCancelOrderMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.tradeflow.order.ModifyOrderRequest,
      com.tradeflow.order.ModifyOrderResponse> getModifyOrderMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ModifyOrder",
      requestType = com.tradeflow.order.ModifyOrderRequest.class,
      responseType = com.tradeflow.order.ModifyOrderResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.tradeflow.order.ModifyOrderRequest,
      com.tradeflow.order.ModifyOrderResponse> getModifyOrderMethod() {
    io.grpc.MethodDescriptor<com.tradeflow.order.ModifyOrderRequest, com.tradeflow.order.ModifyOrderResponse> getModifyOrderMethod;
    if ((getModifyOrderMethod = OrderServiceGrpc.getModifyOrderMethod) == null) {
      synchronized (OrderServiceGrpc.class) {
        if ((getModifyOrderMethod = OrderServiceGrpc.getModifyOrderMethod) == null) {
          OrderServiceGrpc.getModifyOrderMethod = getModifyOrderMethod =
              io.grpc.MethodDescriptor.<com.tradeflow.order.ModifyOrderRequest, com.tradeflow.order.ModifyOrderResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ModifyOrder"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  com.tradeflow.order.ModifyOrderRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  com.tradeflow.order.ModifyOrderResponse.getDefaultInstance()))
              .setSchemaDescriptor(new OrderServiceMethodDescriptorSupplier("ModifyOrder"))
              .build();
        }
      }
    }
    return getModifyOrderMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.tradeflow.order.SubscribeTradesRequest,
      com.tradeflow.order.TradeUpdate> getSubscribeTradesMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SubscribeTrades",
      requestType = com.tradeflow.order.SubscribeTradesRequest.class,
      responseType = com.tradeflow.order.TradeUpdate.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.tradeflow.order.SubscribeTradesRequest,
      com.tradeflow.order.TradeUpdate> getSubscribeTradesMethod() {
    io.grpc.MethodDescriptor<com.tradeflow.order.SubscribeTradesRequest, com.tradeflow.order.TradeUpdate> getSubscribeTradesMethod;
    if ((getSubscribeTradesMethod = OrderServiceGrpc.getSubscribeTradesMethod) == null) {
      synchronized (OrderServiceGrpc.class) {
        if ((getSubscribeTradesMethod = OrderServiceGrpc.getSubscribeTradesMethod) == null) {
          OrderServiceGrpc.getSubscribeTradesMethod = getSubscribeTradesMethod =
              io.grpc.MethodDescriptor.<com.tradeflow.order.SubscribeTradesRequest, com.tradeflow.order.TradeUpdate>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SubscribeTrades"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  com.tradeflow.order.SubscribeTradesRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  com.tradeflow.order.TradeUpdate.getDefaultInstance()))
              .setSchemaDescriptor(new OrderServiceMethodDescriptorSupplier("SubscribeTrades"))
              .build();
        }
      }
    }
    return getSubscribeTradesMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static OrderServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<OrderServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<OrderServiceStub>() {
        @java.lang.Override
        public OrderServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new OrderServiceStub(channel, callOptions);
        }
      };
    return OrderServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static OrderServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<OrderServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<OrderServiceBlockingStub>() {
        @java.lang.Override
        public OrderServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new OrderServiceBlockingStub(channel, callOptions);
        }
      };
    return OrderServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static OrderServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<OrderServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<OrderServiceFutureStub>() {
        @java.lang.Override
        public OrderServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new OrderServiceFutureStub(channel, callOptions);
        }
      };
    return OrderServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     */
    default void submitOrder(com.tradeflow.order.SubmitOrderRequest request,
        io.grpc.stub.StreamObserver<com.tradeflow.order.SubmitOrderResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSubmitOrderMethod(), responseObserver);
    }

    /**
     */
    default void getOrderBook(com.tradeflow.order.GetOrderBookRequest request,
        io.grpc.stub.StreamObserver<com.tradeflow.order.GetOrderBookResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetOrderBookMethod(), responseObserver);
    }

    /**
     */
    default void cancelOrder(com.tradeflow.order.CancelOrderRequest request,
        io.grpc.stub.StreamObserver<com.tradeflow.order.CancelOrderResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCancelOrderMethod(), responseObserver);
    }

    /**
     */
    default void modifyOrder(com.tradeflow.order.ModifyOrderRequest request,
        io.grpc.stub.StreamObserver<com.tradeflow.order.ModifyOrderResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getModifyOrderMethod(), responseObserver);
    }

    /**
     */
    default void subscribeTrades(com.tradeflow.order.SubscribeTradesRequest request,
        io.grpc.stub.StreamObserver<com.tradeflow.order.TradeUpdate> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSubscribeTradesMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service OrderService.
   */
  public static abstract class OrderServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return OrderServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service OrderService.
   */
  public static final class OrderServiceStub
      extends io.grpc.stub.AbstractAsyncStub<OrderServiceStub> {
    private OrderServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected OrderServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new OrderServiceStub(channel, callOptions);
    }

    /**
     */
    public void submitOrder(com.tradeflow.order.SubmitOrderRequest request,
        io.grpc.stub.StreamObserver<com.tradeflow.order.SubmitOrderResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSubmitOrderMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getOrderBook(com.tradeflow.order.GetOrderBookRequest request,
        io.grpc.stub.StreamObserver<com.tradeflow.order.GetOrderBookResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetOrderBookMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void cancelOrder(com.tradeflow.order.CancelOrderRequest request,
        io.grpc.stub.StreamObserver<com.tradeflow.order.CancelOrderResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getCancelOrderMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void modifyOrder(com.tradeflow.order.ModifyOrderRequest request,
        io.grpc.stub.StreamObserver<com.tradeflow.order.ModifyOrderResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getModifyOrderMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void subscribeTrades(com.tradeflow.order.SubscribeTradesRequest request,
        io.grpc.stub.StreamObserver<com.tradeflow.order.TradeUpdate> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getSubscribeTradesMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service OrderService.
   */
  public static final class OrderServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<OrderServiceBlockingStub> {
    private OrderServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected OrderServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new OrderServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public com.tradeflow.order.SubmitOrderResponse submitOrder(com.tradeflow.order.SubmitOrderRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSubmitOrderMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.tradeflow.order.GetOrderBookResponse getOrderBook(com.tradeflow.order.GetOrderBookRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetOrderBookMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.tradeflow.order.CancelOrderResponse cancelOrder(com.tradeflow.order.CancelOrderRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getCancelOrderMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.tradeflow.order.ModifyOrderResponse modifyOrder(com.tradeflow.order.ModifyOrderRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getModifyOrderMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.tradeflow.order.TradeUpdate> subscribeTrades(
        com.tradeflow.order.SubscribeTradesRequest request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getSubscribeTradesMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service OrderService.
   */
  public static final class OrderServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<OrderServiceFutureStub> {
    private OrderServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected OrderServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new OrderServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.tradeflow.order.SubmitOrderResponse> submitOrder(
        com.tradeflow.order.SubmitOrderRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSubmitOrderMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.tradeflow.order.GetOrderBookResponse> getOrderBook(
        com.tradeflow.order.GetOrderBookRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetOrderBookMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.tradeflow.order.CancelOrderResponse> cancelOrder(
        com.tradeflow.order.CancelOrderRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getCancelOrderMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.tradeflow.order.ModifyOrderResponse> modifyOrder(
        com.tradeflow.order.ModifyOrderRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getModifyOrderMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_SUBMIT_ORDER = 0;
  private static final int METHODID_GET_ORDER_BOOK = 1;
  private static final int METHODID_CANCEL_ORDER = 2;
  private static final int METHODID_MODIFY_ORDER = 3;
  private static final int METHODID_SUBSCRIBE_TRADES = 4;

  private static final class MethodHandlers<Req, Resp> implements
      io.grpc.stub.ServerCalls.UnaryMethod<Req, Resp>,
      io.grpc.stub.ServerCalls.ServerStreamingMethod<Req, Resp>,
      io.grpc.stub.ServerCalls.ClientStreamingMethod<Req, Resp>,
      io.grpc.stub.ServerCalls.BidiStreamingMethod<Req, Resp> {
    private final AsyncService serviceImpl;
    private final int methodId;

    MethodHandlers(AsyncService serviceImpl, int methodId) {
      this.serviceImpl = serviceImpl;
      this.methodId = methodId;
    }

    @java.lang.Override
    @java.lang.SuppressWarnings("unchecked")
    public void invoke(Req request, io.grpc.stub.StreamObserver<Resp> responseObserver) {
      switch (methodId) {
        case METHODID_SUBMIT_ORDER:
          serviceImpl.submitOrder((com.tradeflow.order.SubmitOrderRequest) request,
              (io.grpc.stub.StreamObserver<com.tradeflow.order.SubmitOrderResponse>) responseObserver);
          break;
        case METHODID_GET_ORDER_BOOK:
          serviceImpl.getOrderBook((com.tradeflow.order.GetOrderBookRequest) request,
              (io.grpc.stub.StreamObserver<com.tradeflow.order.GetOrderBookResponse>) responseObserver);
          break;
        case METHODID_CANCEL_ORDER:
          serviceImpl.cancelOrder((com.tradeflow.order.CancelOrderRequest) request,
              (io.grpc.stub.StreamObserver<com.tradeflow.order.CancelOrderResponse>) responseObserver);
          break;
        case METHODID_MODIFY_ORDER:
          serviceImpl.modifyOrder((com.tradeflow.order.ModifyOrderRequest) request,
              (io.grpc.stub.StreamObserver<com.tradeflow.order.ModifyOrderResponse>) responseObserver);
          break;
        case METHODID_SUBSCRIBE_TRADES:
          serviceImpl.subscribeTrades((com.tradeflow.order.SubscribeTradesRequest) request,
              (io.grpc.stub.StreamObserver<com.tradeflow.order.TradeUpdate>) responseObserver);
          break;
        default:
          throw new AssertionError();
      }
    }

    @java.lang.Override
    @java.lang.SuppressWarnings("unchecked")
    public io.grpc.stub.StreamObserver<Req> invoke(
        io.grpc.stub.StreamObserver<Resp> responseObserver) {
      switch (methodId) {
        default:
          throw new AssertionError();
      }
    }
  }

  public static final io.grpc.ServerServiceDefinition bindService(AsyncService service) {
    return io.grpc.ServerServiceDefinition.builder(getServiceDescriptor())
        .addMethod(
          getSubmitOrderMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.tradeflow.order.SubmitOrderRequest,
              com.tradeflow.order.SubmitOrderResponse>(
                service, METHODID_SUBMIT_ORDER)))
        .addMethod(
          getGetOrderBookMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.tradeflow.order.GetOrderBookRequest,
              com.tradeflow.order.GetOrderBookResponse>(
                service, METHODID_GET_ORDER_BOOK)))
        .addMethod(
          getCancelOrderMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.tradeflow.order.CancelOrderRequest,
              com.tradeflow.order.CancelOrderResponse>(
                service, METHODID_CANCEL_ORDER)))
        .addMethod(
          getModifyOrderMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.tradeflow.order.ModifyOrderRequest,
              com.tradeflow.order.ModifyOrderResponse>(
                service, METHODID_MODIFY_ORDER)))
        .addMethod(
          getSubscribeTradesMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.tradeflow.order.SubscribeTradesRequest,
              com.tradeflow.order.TradeUpdate>(
                service, METHODID_SUBSCRIBE_TRADES)))
        .build();
  }

  private static abstract class OrderServiceBaseDescriptorSupplier
      implements io.grpc.protobuf.ProtoFileDescriptorSupplier, io.grpc.protobuf.ProtoServiceDescriptorSupplier {
    OrderServiceBaseDescriptorSupplier() {}

    @java.lang.Override
    public com.google.protobuf.Descriptors.FileDescriptor getFileDescriptor() {
      return com.tradeflow.order.OrderServiceOuterClass.getDescriptor();
    }

    @java.lang.Override
    public com.google.protobuf.Descriptors.ServiceDescriptor getServiceDescriptor() {
      return getFileDescriptor().findServiceByName("OrderService");
    }
  }

  private static final class OrderServiceFileDescriptorSupplier
      extends OrderServiceBaseDescriptorSupplier {
    OrderServiceFileDescriptorSupplier() {}
  }

  private static final class OrderServiceMethodDescriptorSupplier
      extends OrderServiceBaseDescriptorSupplier
      implements io.grpc.protobuf.ProtoMethodDescriptorSupplier {
    private final String methodName;

    OrderServiceMethodDescriptorSupplier(String methodName) {
      this.methodName = methodName;
    }

    @java.lang.Override
    public com.google.protobuf.Descriptors.MethodDescriptor getMethodDescriptor() {
      return getServiceDescriptor().findMethodByName(methodName);
    }
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (OrderServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .setSchemaDescriptor(new OrderServiceFileDescriptorSupplier())
              .addMethod(getSubmitOrderMethod())
              .addMethod(getGetOrderBookMethod())
              .addMethod(getCancelOrderMethod())
              .addMethod(getModifyOrderMethod())
              .addMethod(getSubscribeTradesMethod())
              .build();
        }
      }
    }
    return result;
  }
}
