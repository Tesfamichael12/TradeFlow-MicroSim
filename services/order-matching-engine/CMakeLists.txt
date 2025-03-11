cmake_minimum_required(VERSION 3.10)

project(OrderMatchingEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})

include_directories(include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

## Generate C++ sources from proto using protoc and grpc_cpp_plugin
set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/proto/order_service.proto)
set(GENERATED_PROTO_SRC ${CMAKE_CURRENT_BINARY_DIR}/order_service.pb.cc)
set(GENERATED_PROTO_HDR ${CMAKE_CURRENT_BINARY_DIR}/order_service.pb.h)
set(GENERATED_GRPC_SRC ${CMAKE_CURRENT_BINARY_DIR}/order_service.grpc.pb.cc)
set(GENERATED_GRPC_HDR ${CMAKE_CURRENT_BINARY_DIR}/order_service.grpc.pb.h)

find_program(PROTOC_EXECUTABLE protoc)
find_program(GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)

if(NOT PROTOC_EXECUTABLE)
    message(FATAL_ERROR "protoc not found. Please install protoc (protobuf-compiler).")
endif()

if(NOT GRPC_CPP_PLUGIN_EXECUTABLE)
    message(WARNING "grpc_cpp_plugin not found. gRPC C++ plugin missing; grpc sources may not be generated.")
endif()

add_custom_command(
    OUTPUT ${GENERATED_PROTO_SRC} ${GENERATED_PROTO_HDR}
    COMMAND ${PROTOC_EXECUTABLE} --cpp_out=${CMAKE_CURRENT_BINARY_DIR} -I ${CMAKE_CURRENT_SOURCE_DIR}/proto ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating protobuf C++ sources"
)

if(GRPC_CPP_PLUGIN_EXECUTABLE)
    add_custom_command(
        OUTPUT ${GENERATED_GRPC_SRC} ${GENERATED_GRPC_HDR}
        COMMAND ${PROTOC_EXECUTABLE} --grpc_out=${CMAKE_CURRENT_BINARY_DIR} --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE} -I ${CMAKE_CURRENT_SOURCE_DIR}/proto ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Generating gRPC C++ sources"
    )
else()
    # Fallback: create empty files to avoid build break; user should install grpc_cpp_plugin
    file(WRITE ${GENERATED_GRPC_SRC} "// missing grpc_cpp_plugin; please install grpc tools to generate this file\n")
    file(WRITE ${GENERATED_GRPC_HDR} "// missing grpc_cpp_plugin; please install grpc tools to generate this file\n")
endif()

set(PROTO_SRCS ${GENERATED_PROTO_SRC} ${GENERATED_GRPC_SRC})
set(PROTO_HDRS ${GENERATED_PROTO_HDR} ${GENERATED_GRPC_HDR})

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/order_matching/Matcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/order_matching/OrderBook.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/order_matching/Order.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/order_matching/main.cpp
)

add_executable(order-matching-engine ${SOURCES} ${PROTO_SRCS} ${GRPC_SRCS})

# Try to find system libraries for gRPC; fall back to linking against -lgrpc++ -lgrpc
find_library(GRPCPP_LIB NAMES grpc++ PATHS /usr/lib /usr/local/lib)
find_library(GRPC_LIB NAMES grpc PATHS /usr/lib /usr/local/lib)

if(GRPCPP_LIB AND GRPC_LIB)
    message(STATUS "Found gRPC libraries: ${GRPCPP_LIB}; ${GRPC_LIB}")
    target_link_libraries(order-matching-engine
        ${PROTOBUF_LIBRARIES}
        ${GRPCPP_LIB}
        ${GRPC_LIB}
    )
else()
    message(STATUS "gRPC CMake config not found; linking with grpc++ and grpc from system libs")
    target_link_libraries(order-matching-engine
        ${PROTOBUF_LIBRARIES}
        grpc++
        grpc
    )
endif()

target_include_directories(order-matching-engine PRIVATE ${Protobuf_INCLUDE_DIRS})
target_include_directories(order-matching-engine PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

enable_testing()

# Integration test target: builds the binary then runs the integration script
add_custom_target(integration-test
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target order-matching-engine -- -j${CMAKE_BUILD_PARALLEL_LEVEL}
    COMMAND ${CMAKE_COMMAND} -E env "PYTHONUNBUFFERED=1" bash ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/integration_test.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS order-matching-engine
)

add_test(NAME integration_test COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/integration_test.sh)

## GoogleTest setup for unit tests - prefer system package, otherwise FetchContent
find_package(GTest QUIET)
if(TARGET gtest_main OR TARGET GTest::gtest_main OR GTest_FOUND)
    message(STATUS "GoogleTest found; adding unit tests")
        # Unit test: OrderBook (use GoogleTest)
    add_executable(OrderBook_test tests/unit/OrderBook_test.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/order_matching/OrderBook.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/order_matching/Order.cpp)
    target_include_directories(OrderBook_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR})
    if(TARGET gtest_main)
        target_link_libraries(OrderBook_test PRIVATE gtest_main ${PROTOBUF_LIBRARIES})
    elseif(TARGET GTest::gtest_main)
        target_link_libraries(OrderBook_test PRIVATE GTest::gtest_main ${PROTOBUF_LIBRARIES})
    else()
        target_link_libraries(OrderBook_test PRIVATE GTest::GTest ${PROTOBUF_LIBRARIES})
    endif()
    include(GoogleTest)
    gtest_discover_tests(OrderBook_test)
else()
    message(WARNING "GoogleTest not found; skipping unit tests. To enable tests, install GTest or allow network access for FetchContent.")
endif()

## Prefer system-installed benchmark and nlohmann_json; fall back to FetchContent
find_package(benchmark QUIET)
find_package(nlohmann_json QUIET)

if(NOT benchmark)
    message(WARNING "Google Benchmark not found. Benchmarks will be skipped. Install libbenchmark-dev or the benchmark CMake package to enable benchmarks.")
endif()

if(NOT nlohmann_json)
    message(WARNING "nlohmann_json not found. Replay runner will be skipped. Install nlohmann-json3-dev or the package to enable replay functionality.")
endif()

# Fallback: if the CMake config packages aren't available but system headers/libs are installed
# try to detect them and create simple imported targets so guarded targets can be created.
if(NOT nlohmann_json)
    find_path(NLOHMANN_JSON_INCLUDE_DIR NAMES nlohmann/json.hpp PATHS /usr/include /usr/local/include /usr/include/x86_64-linux-gnu)
    if(NLOHMANN_JSON_INCLUDE_DIR)
        if(NOT TARGET nlohmann_json::nlohmann_json)
            message(STATUS "Found nlohmann/json.hpp at ${NLOHMANN_JSON_INCLUDE_DIR}; creating imported target nlohmann_json::nlohmann_json")
            add_library(nlohmann_json::nlohmann_json INTERFACE IMPORTED)
            set_target_properties(nlohmann_json::nlohmann_json PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${NLOHMANN_JSON_INCLUDE_DIR}")
        else()
            message(STATUS "nlohmann_json::nlohmann_json target already exists; skipping creation")
        endif()
        set(nlohmann_json TRUE)
    endif()
endif()

if(NOT benchmark)
    # Try to find benchmark header and library
    find_path(BENCHMARK_INCLUDE_DIR NAMES benchmark/benchmark.h PATHS /usr/include /usr/local/include /usr/include/x86_64-linux-gnu)
    find_library(BENCHMARK_LIB NAMES benchmark benchmark_main PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu)
    if(BENCHMARK_LIB AND BENCHMARK_INCLUDE_DIR)
        if(NOT TARGET benchmark::benchmark)
            message(STATUS "Found Google Benchmark library at ${BENCHMARK_LIB} and headers at ${BENCHMARK_INCLUDE_DIR}; creating imported target benchmark::benchmark")
            add_library(benchmark::benchmark UNKNOWN IMPORTED)
            set_target_properties(benchmark::benchmark PROPERTIES IMPORTED_LOCATION "${BENCHMARK_LIB}" INTERFACE_INCLUDE_DIRECTORIES "${BENCHMARK_INCLUDE_DIR}")
        else()
            message(STATUS "benchmark::benchmark target already exists; skipping creation")
        endif()
        set(benchmark TRUE)
    endif()
endif()

if(benchmark)
    add_executable(order_bench src/benchmarks/OrderBench.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/order_matching/OrderBook.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/order_matching/Order.cpp)
    target_include_directories(order_bench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries(order_bench PRIVATE benchmark::benchmark)
else()
    message(STATUS "Skipping creation of order_bench target because benchmark was not found")
endif()

# Replay runner (CLI) - placed under repo-level tools
if(nlohmann_json)
    # tools/replay/ReplayRunner.cpp lives at repo_root/tools/replay/ReplayRunner.cpp
    set(REPLAY_RUNNER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../tools/replay/ReplayRunner.cpp)
    if(EXISTS ${REPLAY_RUNNER_SRC})
        add_executable(replay_runner ${REPLAY_RUNNER_SRC} ${CMAKE_CURRENT_SOURCE_DIR}/src/order_matching/OrderBook.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/order_matching/Order.cpp)
        target_include_directories(replay_runner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../../tools/replay)
        target_link_libraries(replay_runner PRIVATE nlohmann_json::nlohmann_json)
    else()
        message(WARNING "Replay source ${REPLAY_RUNNER_SRC} not found; skipping replay_runner target")
    endif()
else()
    message(STATUS "Skipping creation of replay_runner because nlohmann_json was not found")
endif()

## Deterministic replay unit test
# Use the same GTest detection logic as above (check targets or GTest_FOUND)
if((TARGET gtest_main OR TARGET GTest::gtest_main OR GTest_FOUND) AND nlohmann_json)
    add_executable(replay_test tests/unit/replay_test.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/order_matching/OrderBook.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/order_matching/Order.cpp)
    target_include_directories(replay_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR})
    if(TARGET gtest_main)
        target_link_libraries(replay_test PRIVATE gtest_main nlohmann_json::nlohmann_json)
    elseif(TARGET GTest::gtest_main)
        target_link_libraries(replay_test PRIVATE GTest::gtest_main nlohmann_json::nlohmann_json)
    else()
        target_link_libraries(replay_test PRIVATE GTest::GTest nlohmann_json::nlohmann_json)
    endif()
    # Allow the test to find testdata relative to the source dir regardless of CTest working dir
    target_compile_definitions(replay_test PRIVATE TEST_SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}")
    include(GoogleTest)
    gtest_discover_tests(replay_test)
else()
    message(STATUS "Skipping creation of replay_test because GTest or nlohmann_json not available")
endif()


