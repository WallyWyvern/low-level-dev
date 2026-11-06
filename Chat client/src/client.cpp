// Barebones ASIO chat client.
//
// Console-only for now (no SFML/ImGui) so the networking layer can be
// verified in isolation. ChatClient is shared between GUI/Non-GUI versions
// Io_context runs on a background thread, incoming frames land in a thread-safe queue 
// that the "UI" (here, a print loop) drains.
//
// Build: CMakeLists from root folder (adds dependencies & src from there).
// Usage: chat_client <host> <port> <name>

#include "ChatClient.hpp"

#include <iostream>

using asio::ip::tcp;
using namespace std::chrono_literals;

int main(int argc, char* argv[]) {
    const std::string host = argc > 1 ? argv[1] : "127.0.0.1";
    const std::string port = argc > 2 ? argv[2] : "5555";
    std::string password;
    std::string username;
    std::atomic<bool> isBanned{false}; //for thread safety according to gemini,

    std::cout << "Please enter a username, if it does not exist it will be created: " << std::endl;
    std::cin >> username;
    std::cout << "Please enter your password: " << std::endl;
    std::cin >> password;

    asio::io_context io;
    ChatClient client{io};
    client.connect(host, port, username, password);

    // Run the io_context on its own thread so stdin can be read on main().
    auto work_guard = asio::make_work_guard(io);
    std::thread net_thread{[&io] { io.run(); }};

    // Background printer: drains incoming messages as they arrive.
    std::thread print_thread{
        [&client, &isBanned] {
            std::string msg;
            while (true) {
                if (client.poll_message(msg)) {
                    auto barPos = msg.find('|');
                    if (msg.empty() || barPos == std::string::npos) return;

                    const std::string& type = msg.substr(0, barPos);
                    const std::string& body = msg.substr(barPos + 1);

                    if (type == "WELCOME") {
                        std::cout << "[INFO] connected as " << body << "\n";
                    } else if (type == "MSG") {
                        std::cout << body << "\n";
                    } else if (type=="CONNECTIONERROR") {
                        std::cout << "[Connection Error]" + body << "\n";
                        client.close();
                    } else if (type== "CMDERROR")
                    {
                        std::cout << "[Command error]" + body << "\n";
                    } else if (type== "BAN")
                    {
                        std::cout << "[Connection Error]" + body << "\n";
                        isBanned = true;
                        client.close();
                    }
                } else {
                    std::this_thread::sleep_for(20ms);
                }
            }
        }
    };

    std::cout << "Type a message and press Enter to send. Ctrl+C to quit.\n";
    std::string line;
    while (std::getline(std::cin, line)) {
        if (!client.last_error().empty() && !client.is_connected()) {
            std::cout << "Error: " << client.last_error().c_str() << "\n";
            // Simple reconnect loop
            while (!client.is_connected() & !isBanned) {
                std::cout << "[attempting reconnect]\n";
                client.connect(host, port, username, password);
                std::this_thread::sleep_for(5s);
            }
        }
        if (line.starts_with('/')) {
            auto commandEnd = line.find(' ');
            if (commandEnd != std::string::npos) {
                auto command = line.substr(1, commandEnd - 1);
                std::transform(command.begin(), command.end(), command.begin(), ::toupper);
                client.send({command + '|' + line.substr(commandEnd + 1)});
            }
        } else if (!line.empty()) {
            client.send_chat_message(line);
        }
    }

    work_guard.reset();
    io.stop();
    net_thread.join();
    print_thread.detach(); // it'll exit once poll_message keeps failing after close
    return 0;
}
