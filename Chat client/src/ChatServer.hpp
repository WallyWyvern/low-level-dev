#pragma once

#include "Session.hpp"
#include "user.h"

#include <asio.hpp>
#include <map>
#include <string>
#include <unordered_map>
#include <fstream>

using SessionPtr = std::shared_ptr<Session>;
using asio::ip::tcp;

// ChatServer: accepts connections and routes messages between sessions
class ChatServer {
public:
    ChatServer(asio::io_context& io, unsigned short port);
    ~ChatServer();

private:
    void do_accept();
    void on_message(const SessionPtr& s, const std::string& msg);
    void on_close(const SessionPtr& s);
    void broadcast_except(const std::string& sender_id, const std::string& msg);
    void load_users();
    void save_users();

private:
    std::string file = "users.txt";
    std::ifstream ifs;
    std::ofstream ofs;
    tcp::acceptor acceptor_;
    std::unordered_map<std::string, SessionPtr> sessions_{};
    std::map<std::string, user> users{};
    int next_id_ = 0;
};
