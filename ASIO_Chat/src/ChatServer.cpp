#include "ChatServer.hpp"

#include <iostream>

using asio::ip::tcp;

ChatServer::ChatServer(asio::io_context& io, const unsigned short port)
    : acceptor_{io, tcp::endpoint{tcp::v4(), port}} {

    std::cout << "Chat server listening on port " << port << "\n";
    do_accept();
}

void ChatServer::do_accept() {
    std::cout << "Waiting for client..." << std::endl;
    acceptor_.async_accept([this](const std::error_code ec, tcp::socket socket) {
        if (!ec) {
            auto id = "client-" + std::to_string(next_id_++);
            std::cout << "Client connected from " << socket.remote_endpoint() << ", id=" << id << "\n";
            auto session = std::make_shared<Session>(std::move(socket),
                                                     [this](const SessionPtr& s, const std::string& msg) { on_message(s, msg); },
                                                     [this](const SessionPtr& s) { on_close(s); });

            session->set_id(id);
            sessions_[session->id()] = session;
            session->start();
        }
        do_accept(); // keep accepting further connections
    });
}

void ChatServer::on_message(const SessionPtr& s, const std::string& msg) {
    auto barPos = msg.find('|');
    if (msg.empty() || barPos == std::string::npos) return;

    const std::string& type = msg.substr(0, barPos);
    const std::string& body = msg.substr(barPos + 1);

    if (type == "CONNECT") {
        s->send("WELCOME|" + s->id());
        broadcast_except(s->id(), "MSG|[INFO] " + s->id() + " connected");
    } else if (type == "MSG") {
        std::cout << s->id() << (!s->nickname().empty() ? "[" + s->nickname() + "]" : "") << ": " << body << "\n";
        broadcast_except(s->id(), {"MSG|[" + (!s->nickname().empty() ? s->nickname() : s->id()) + "] " + body});
    } /*else if (type == "") {

    }*/
}

void ChatServer::on_close(const SessionPtr& s) {
    std::cout << "Client disconnected: " << s->id() << "\n";
    broadcast_except(s->id(), "MSG|[INFO] " + s->id() + " disconnected");
    sessions_.erase(s->id());
}

void ChatServer::broadcast_except(const std::string& sender_id, const std::string& msg) {
    for (auto& [id, session]: sessions_) {
        if (id != sender_id) session->send(msg);
    }
}
