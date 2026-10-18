// Barebones ASIO chat server.
//
// Supported message types (first frame is the type):
//   ["CONNECT"]                     -> server replies ["WELCOME", "<id>"]
//   ["MSG", "<text>"]               -> server broadcasts ["MSG", "<id>", "<text>"] to everyone else
//
// Build: CMakeLists from root folder (adds dependencies & src from there).

#include <iostream>

#include "ChatServer.hpp"

int main(int argc, char* argv[]) {
    unsigned short port = 5555;
    if (argc > 1) port = static_cast<unsigned short>(std::atoi(argv[1]));

    try {
        asio::io_context io;
        ChatServer server(io, port);
        io.run(); // blocks; runs the async event loop
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
