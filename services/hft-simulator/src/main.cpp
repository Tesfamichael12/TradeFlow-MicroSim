#include <iostream>
#include <csignal>
#include <grpcpp/grpcpp.h>
#include "grpc/server.h" 

namespace {
    volatile std::sig_atomic_t gSignalStatus = 0;
}

void signal_handler(int signal) {
    gSignalStatus = signal;
}

int main(int argc, char** argv) {
    try {
        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);

        const std::string server_address("0.0.0.0:50052");
        
        std::cout << "WarpSpeed HFT Simulator" << std::endl;
        std::cout << "Starting gRPC server on " << server_address << std::endl;

        // Create and start the server
        warpspeed::HFTServer server(server_address);
        
        // Launch server in a separate thread so we can handle signals
        std::thread server_thread([&server]() {
            server.start();
        });

        // Wait for shutdown signal
        while (gSignalStatus == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Graceful shutdown
        std::cout << "\nShutting down server..." << std::endl;
        server.stop();
        
        if (server_thread.joinable()) {
            server_thread.join();
        }

        std::cout << "Server shutdown complete." << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}