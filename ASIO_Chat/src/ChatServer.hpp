#pragma once

#include "Session.hpp"

#include <asio.hpp>
#include <string>
#include <unordered_map>

using SessionPtr = std::shared_ptr<Session>;
using asio::ip::tcp;

// ChatServer: accepts connections and routes messages between sessions
class ChatServer {
public:
    ChatServer(asio::io_context& io, unsigned short port);

private:
    void do_accept();
    void on_message(const SessionPtr& s, const std::string& msg);
    void on_close(const SessionPtr& s);
    void broadcast_except(const std::string& sender_id, const std::string& msg);

private:
    tcp::acceptor acceptor_;
    std::unordered_map<std::string, SessionPtr> sessions_{};
    int next_id_ = 0;
};
