cmake_minimum_required(VERSION 3.15)
project(WarpSpeedHFT VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find Protobuf and gRPC using modern CMake targets
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# --- Windows-Specific Tool Path Fix ---
# If we are on Windows and the find_package commands didn't automatically
# find the executables, we can set them manually here.
if(WIN32 AND NOT Protobuf_PROTOC_EXECUTABLE)
    message(STATUS "Protobuf_PROTOC_EXECUTABLE not found, setting manually for Windows.")
    # IMPORTANT: VERIFY THIS PATH ON YOUR COMPUTER!
    set(Protobuf_PROTOC_EXECUTABLE "C:/msys64/mingw64/bin/protoc.exe")
endif()

if(WIN32 AND NOT gRPC_CPP_PLUGIN_EXECUTABLE)
    message(STATUS "gRPC_CPP_PLUGIN_EXECUTABLE not found, setting manually for Windows.")
    # IMPORTANT: VERIFY THIS PATH ON YOUR COMPUTER!
    set(gRPC_CPP_PLUGIN_EXECUTABLE "C:/msys64/mingw64/bin/grpc_cpp_plugin.exe")
endif()

# Define the location of the .proto file
set(PROTO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/api/warpspeed.proto)
get_filename_component(PROTO_FILE_DIR "${PROTO_FILES}" DIRECTORY)

# Generate gRPC and Protobuf C++ files
add_custom_command(
    OUTPUT 
        "${CMAKE_CURRENT_BINARY_DIR}/warpspeed.pb.cc"
        "${CMAKE_CURRENT_BINARY_DIR}/warpspeed.pb.h"
        "${CMAKE_CURRENT_BINARY_DIR}/warpspeed.grpc.pb.cc"
        "${CMAKE_CURRENT_BINARY_DIR}/warpspeed.grpc.pb.h"
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS 
        --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${PROTO_FILE_DIR}"
        --plugin=protoc-gen-grpc="${gRPC_CPP_PLUGIN_EXECUTABLE}"
        "${PROTO_FILES}"
    DEPENDS "${PROTO_FILES}"
)

# Create a library for the generated code
add_library(warpspeed_proto
    ${CMAKE_CURRENT_BINARY_DIR}/warpspeed.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/warpspeed.grpc.pb.cc
)

target_include_directories(warpspeed_proto PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Link the generated library against the gRPC and Protobuf libraries
target_link_libraries(warpspeed_proto PUBLIC
    protobuf::libprotobuf
    gRPC::grpc++
)

# Collect all application source files
file(GLOB_RECURSE CORE_SOURCES "src/core/*.cpp")
file(GLOB_RECURSE GRPC_SOURCES "src/grpc/*.cpp")

# Create the main server executable
add_executable(warpspeed_server
    src/main.cpp
    ${CORE_SOURCES}
    ${GRPC_SOURCES}
)

target_include_directories(warpspeed_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(warpspeed_server PRIVATE
    warpspeed_proto
)

# === Tests ===
option(BUILD_TESTS "Build the gRPC test client" ON)

if (BUILD_TESTS)
    add_executable(grpc_client_test
        tests/core/grpc_client_test.cpp
    )

    target_include_directories(grpc_client_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR} 
    )

    # The test client also needs to link against the generated code and gRPC
    target_link_libraries(grpc_client_test PRIVATE
        warpspeed_proto
    )
endif()