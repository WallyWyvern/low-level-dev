add_executable(chat_server
        server.cpp
        Session.hpp Session.cpp
        ChatServer.hpp ChatServer.cpp)
target_link_libraries(chat_server PRIVATE asio::asio)

# --- console Client ----------------------------
add_executable(chat_client
        client.cpp
        ChatClient.hpp ChatClient.cpp
        Session.hpp Session.cpp)
target_link_libraries(chat_client PRIVATE asio::asio)

# --- sfml/IMGui client -------------------------
add_executable(chat_client_gui
        client_gui.cpp
        ChatClient.hpp ChatClient.cpp)
target_link_libraries(chat_client_gui PRIVATE
        asio::asio
        ImGui-SFML::ImGui-SFML
        sfml-graphics sfml-window sfml-system
)

# On MINGW toolchains needs extra link flags
if (MINGW)
    target_link_libraries(chat_server PRIVATE ws2_32 wsock32 bcrypt)
    target_link_libraries(chat_client PRIVATE ws2_32 wsock32 bcrypt)
    target_link_libraries(chat_client_gui PRIVATE ws2_32 wsock32 bcrypt)
endif ()