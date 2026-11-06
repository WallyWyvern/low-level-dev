// Barebones SFML + ImGui-SFML chat client.
//
// Uses the "poll io_context once per frame" integration pattern:
//  no background thread, no locking. ChatClient::send() still goes
// through asio::post() so it would also be safe to call from another
// thread later.
//
// Build: CMakeLists from root folder (adds dependencies & src from there).

#include "ChatClient.hpp"

#include <imgui-SFML.h>
#include <imgui.h>

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>

using asio::ip::tcp;

// Chat log entry for the UI
struct LogLine {
    std::string author;
    std::string text;
};

int main() {
    asio::io_context io;
    ChatClient client(io);
    auto work_guard = asio::make_work_guard(io);

    sf::RenderWindow window(sf::VideoMode({800, 600}), "ASIO Chat Client");
    window.setFramerateLimit(60);
    if (!ImGui::SFML::Init(window)) return -1;

    sf::Clock delta_clock;

    // UI state
    char host_buf[128] = "127.0.0.1";
    char port_buf[16] = "5555";
    char user_buf[128];
    char pass_buf[128];
    char message_buf[512] = "";
    std::vector<LogLine> log;
    bool auto_scroll = true;

    while (window.isOpen()) {
        // Event Polling
        while (const std::optional event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);

            // "close requested" event: we close the window
            if (event->is<sf::Event::Closed>()) window.close();
        }

        // Pump ASIO's async handlers. Non-blocking;
        // returns immediately if there's no completed I/O to process.
        io.poll();

        // Drain anything the network layer received since last frame.
        std::string msg;
        while (client.poll_message(msg)) {

            auto barPos = msg.find('|');
            if (msg.empty() || barPos == std::string::npos) continue;

            const std::string& type = msg.substr(0, barPos);
            const std::string& body = msg.substr(barPos + 1);

            if (type == "WELCOME") {
                log.push_back({"[INFO]", "Connected as " + body});
            } else if (type == "MSG") {
                auto headerEnd = body.find(']');
                if (headerEnd == std::string::npos) {
                    log.push_back({">", body});
                } else {
                    log.push_back({body.substr(0, headerEnd + 1), body.substr(headerEnd + 2)});
                }
            }
        }

        ImGui::SFML::Update(window, delta_clock.restart());

        // TODO: Check if we can inject some kind of default state into the windows (so they don't overlap, and we get a decent layout)
        // --- Connection panel ---
        ImGui::Begin("Connection");
        ImGui::BeginDisabled(client.is_connected() || client.is_connecting());
        ImGui::InputText("Host", host_buf, sizeof(host_buf));
        ImGui::InputText("Port", port_buf, sizeof(port_buf));
        ImGui::InputText("User", user_buf, sizeof(user_buf));
        ImGui::InputText("Pass", pass_buf, sizeof(pass_buf));
        if (ImGui::Button("Connect")) {
            client.connect(host_buf, port_buf, user_buf, pass_buf);
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        if (client.is_connected()) {
            ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.3f, 1.0f), "Connected");
        } else if (client.is_connecting()) {
            ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.3f, 1.0f), "Connecting...");
        } else {
            ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f), "Disconnected");
        }
        if (!client.last_error().empty()) {
            ImGui::TextWrapped("Last error: %s", client.last_error().c_str());
        }
        ImGui::End();

        // --- Chat window ---
        ImGui::Begin("Chat");

        ImGui::BeginChild("ScrollingLog", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()),
                          true, ImGuiWindowFlags_HorizontalScrollbar);
        for (const auto& line: log) {
            if (line.author.empty()) {
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", line.text.c_str());
            } else {
                ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.3f, 1.0f), "%s ", line.author.c_str());
                ImGui::SameLine();
                ImGui::TextWrapped("%s", line.text.c_str());
            }
        }
        if (auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f) {
            ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();

        bool send_now = false;
        ImGui::PushItemWidth(-80);
        if (ImGui::InputText("##message", message_buf, sizeof(message_buf),
                             ImGuiInputTextFlags_EnterReturnsTrue)) {
            send_now = true;
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Send") || send_now) {
            if (client.is_connected() && message_buf[0] != '\0') {
                client.send_chat_message(message_buf);
                log.push_back({"me", message_buf});
                message_buf[0] = '\0';
            }
        }

        ImGui::End();

        window.clear(sf::Color(30, 30, 30));
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}
