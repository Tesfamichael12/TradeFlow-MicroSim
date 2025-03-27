cmake_minimum_required(VERSION 3.14)

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
    message(FATAL_ERROR "grpc_cpp_plugin not found. Please install protobuf-compiler-grpc package.")
endif()

add_custom_command(
    OUTPUT ${GENERATED_PROTO_SRC} ${GENERATED_PROTO_HDR}
    COMMAND ${PROTOC_EXECUTABLE} --cpp_out=${CMAKE_CURRENT_BINARY_DIR} -I ${CMAKE_CURRENT_SOURCE_DIR}/proto ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating protobuf C++ sources"
)

add_custom_command(
    OUTPUT ${GENERATED_GRPC_SRC} ${GENERATED_GRPC_HDR}
    COMMAND ${PROTOC_EXECUTABLE} --grpc_out=${CMAKE_CURRENT_BINARY_DIR} --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE} -I ${CMAKE_CURRENT_SOURCE_DIR}/proto ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating gRPC C++ sources"
)

set(PROTO_SRCS ${GENERATED_PROTO_SRC} ${GENERATED_GRPC_SRC})
set(PROTO_HDRS ${GENERATED_PROTO_HDR} ${GENERATED_GRPC_HDR})

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/order_matching/Matcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/order_matching/OrderBook.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/order_matching/Order.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/order_matching/main.cpp
)

add_executable(order-matching-engine ${SOURCES} ${PROTO_SRCS})

# Link against gRPC (prefer CMake package, then pkg-config, finally manual libs)
find_package(gRPC CONFIG QUIET)
if(gRPC_FOUND)
    message(STATUS "Using gRPC CMake package: linking with gRPC::grpc++")
    set(GRPC_LINK_LIBS gRPC::grpc++)
else()
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(GRPCPP IMPORTED_TARGET QUIET grpc++)
    endif()
    if(TARGET PkgConfig::GRPCPP)
        message(STATUS "Using pkg-config for grpc++")
        set(GRPC_LINK_LIBS PkgConfig::GRPCPP)
    else()
        message(STATUS "gRPC CMake config/pkg-config not found; falling back to manual library lookup")
        find_library(GRPCPP_LIB NAMES grpc++ PATHS /usr/lib /usr/local/lib)
        find_library(GRPC_LIB NAMES grpc PATHS /usr/lib /usr/local/lib)
        find_library(GPR_LIB NAMES gpr PATHS /usr/lib /usr/local/lib)
        if(NOT GRPCPP_LIB OR NOT GRPC_LIB)
            message(FATAL_ERROR "Unable to locate grpc++/grpc libraries. Install libgrpc-dev or provide gRPC CMake package.")
        endif()
        set(GRPC_LINK_LIBS ${GRPCPP_LIB} ${GRPC_LIB})
        if(GPR_LIB)
            list(APPEND GRPC_LINK_LIBS ${GPR_LIB})
        endif()
        foreach(_absl_libname IN ITEMS absl_synchronization absl_time absl_strings absl_base absl_status absl_int128 absl_hash absl_cord)
            find_library(_absl_lib NAMES ${_absl_libname} PATHS /usr/lib /usr/local/lib)
            if(_absl_lib)
                list(APPEND GRPC_LINK_LIBS ${_absl_lib})
            endif()
        endforeach()
    endif()
endif()

set(PROTOBUF_LINK_LIB ${PROTOBUF_LIBRARIES})
if(TARGET protobuf::libprotobuf)
    set(PROTOBUF_LINK_LIB protobuf::libprotobuf)
endif()

target_link_libraries(order-matching-engine
    PRIVATE
        ${PROTOBUF_LINK_LIB}
        ${GRPC_LINK_LIBS}
)

target_include_directories(order-matching-engine PRIVATE ${Protobuf_INCLUDE_DIRS})
target_include_directories(order-matching-engine PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

enable_testing()

# Integration test target: builds the binary then runs the integration script
add_custom_target(integration-test
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target order-matching-engine -- -j${CMAKE_BUILD_PARALLEL_LEVEL}
    COMMAND ${CMAKE_COMMAND} -E env "PYTHONUNBUFFERED=1" "ORDER_MATCHING_ENGINE_BIN=$<TARGET_FILE:order-matching-engine>" bash ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/integration_test.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS order-matching-engine
)

add_test(NAME integration_test COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/integration_test.sh)
set_tests_properties(integration_test PROPERTIES
    ENVIRONMENT "PYTHONUNBUFFERED=1;ORDER_MATCHING_ENGINE_BIN=$<TARGET_FILE:order-matching-engine>"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration
)

## GoogleTest setup for unit tests - prefer system package, otherwise FetchContent
find_package(GTest QUIET)
set(HAVE_GTEST FALSE)

if(TARGET gtest_main OR TARGET GTest::gtest_main OR GTest_FOUND)
    set(HAVE_GTEST TRUE)
else()
    message(STATUS "GoogleTest not found via package; fetching from source")
    include(FetchContent)
    set(INSTALL_GTEST OFF CACHE BOOL "Disable installation for googletest" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    FetchContent_MakeAvailable(googletest)
    if(TARGET gtest_main OR TARGET GTest::gtest_main)
        set(HAVE_GTEST TRUE)
    endif()
endif()

if(HAVE_GTEST)
    message(STATUS "GoogleTest available; adding unit tests")
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
    message(WARNING "GoogleTest could not be located or fetched; skipping unit tests. Install GTest or ensure network access for FetchContent.")
endif()

## Prefer system-installed benchmark and nlohmann_json; fall back to FetchContent
find_package(benchmark QUIET)
find_package(nlohmann_json QUIET)

if(NOT benchmark)
    message(STATUS "Google Benchmark not found via package; attempting manual discovery or FetchContent")
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

if(NOT benchmark)
    message(STATUS "Fetching Google Benchmark via FetchContent")
    include(FetchContent)
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.8.3
    )
    FetchContent_MakeAvailable(googlebenchmark)
    if(TARGET benchmark::benchmark)
        set(benchmark TRUE)
    endif()
endif()

if(NOT benchmark)
    message(WARNING "Google Benchmark could not be located or fetched; benchmarks will be skipped. Install libbenchmark-dev or ensure network access for FetchContent.")
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


