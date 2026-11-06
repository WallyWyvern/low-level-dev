#include "ChatServer.hpp"
#include "util.h"

#include <iostream>

using asio::ip::tcp;

ChatServer::ChatServer(asio::io_context& io, const unsigned short port)
    : acceptor_{io, tcp::endpoint{tcp::v4(), port}} {

    users = std::map<std::string, user>();
    load_users();
    std::cout << "Chat server listening on port " << port << "\n";
    do_accept();
}

ChatServer::~ChatServer()
{
    save_users();
}

void ChatServer::do_accept() {
    std::cout << "Waiting for client..." << std::endl;
    acceptor_.async_accept([this](const std::error_code ec, tcp::socket socket) {
        if (!ec) {
            auto id = "unknown-client";
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
        std::vector<std::string> login = util::split(body, ", ");
        std::transform(login[0].begin(), login[0].end(), login[0].begin(), ::tolower);
        if (login.empty())
        {
            s->send("CONNECTIONERROR| failed to provide username or password");
            return;
        };
        if (!users.contains(login[0]))
        {
            users.insert({login[0], user()});
            users[login[0]].password = login[1];
        }
        if (users[login[0]].password != login[1])
        {
            s->send("CONNECTIONERROR| incorrect password provided");
            return;
        }
        if (users[login[0]].isBanned)
        {
            s->send("BAN| You are banned from this server");
            return;
        }
        users[login[0]].currentSession = s;
        s->set_id(login[0]);
        s->send("WELCOME|" + s->id());
        broadcast_except(s->id(), "MSG|[INFO] " + s->id() + " connected");
        save_users();
    } else if (type == "MSG") {
        if (users[s->id()].isBanned)
        {
            return;
        }
        const std::string fullMsg = "[" + (!s->nickname().empty() ? s->nickname() : s->id()) + "] " + body;
        std::cout << fullMsg << std::endl;
        broadcast_except(s->id(), "MSG|" + fullMsg);
    } else if (type == "NICK") {
        if (users[s->id()].isBanned)
        {
            return;
        }
        const std::string& newNick = body;
        s->set_nickname(newNick);
        const std::string nickMsg = s->id() + " is now known as " + newNick;
        std::cout << nickMsg << std::endl;
        broadcast_except(s->id(), "MSG|" + nickMsg);
    } else if (type == "BAN")
    {
        if (!users[s->id()].isAdmin)
        {
            s->send("CMDERROR| you need admin privileges to execute this command" );
            return;
        }
        if (!users.contains(body))
        {
            s->send("CMDERROR| this user does not exist" );
            return;
        }
        users[body].isBanned = true;
        if (users[body].currentSession != nullptr)
        {
            users[body].currentSession->send("BAN| you have been banned from the server");
        }
        save_users();
    } else if (type == "UNBAN")
    {
        if (!users[s->id()].isAdmin)
        {
            s->send("CMDERROR| you need admin privileges to execute this command" );
            return;
        }
        if (!users.contains(body))
        {
            s->send("CMDERROR| this user does not exist" );
            return;
        }
        users[body].isBanned = false;
        save_users();
    } else if (type == "KICK")
    {
        if (!users[s->id()].isAdmin)
        {
            s->send("CMDERROR| you need admin privileges to execute this command" );
            return;
        }
        if (!users.contains(body))
        {
            s->send("CMDERROR| this user does not exist" );
            return;
        }
        if (users[body].currentSession != nullptr)
        {
            users[body].currentSession->send("CONNECTIONERROR| you have been kicked from the server");
        }
    } else if (type == "DM" || type == "PM" || "WHISPER")
    {
        if (users[s->id()].isBanned)
        {
            return;
        }
        auto spacePos = body.find(' ');
        if (body.empty() || spacePos == std::string::npos)
        {
            s->send("CMDERROR| wrong formatting, use /DM [username] [message]" );
            return;
        }

        const std::string& recipient = body.substr(0, spacePos);
        const std::string& dm = body.substr(spacePos + 1);

        std::string fullMsg = "[" + (!s->nickname().empty() ? s->nickname() : s->id()) + "->" + (!users[recipient].currentSession->nickname().empty() ? users[recipient].currentSession->nickname() : users[recipient].currentSession->id()) + "] " + dm;
        std::cout << fullMsg << std::endl;

        fullMsg = "[TO->" + (!users[recipient].currentSession->nickname().empty() ? users[recipient].currentSession->nickname() : users[recipient].currentSession->id()) + "] " + dm;
        s->send("MSG|" + fullMsg);
        fullMsg = "[FROM->" + (!s->nickname().empty() ? s->nickname() : s->id()) + "] " + dm;
        users[recipient].currentSession->send("MSG|" + fullMsg);

    }
}

void ChatServer::on_close(const SessionPtr& s) {
    std::cout << "Client disconnected: " << s->id() << "\n";
    broadcast_except(s->id(), "MSG|[INFO] " + s->id() + " disconnected");
    sessions_.erase(s->id());
}

void ChatServer::broadcast_except(const std::string& sender_id, const std::string& msg) {
    for (auto& [id, user]: users) {
        if (id != sender_id & user.currentSession != nullptr) user.currentSession->send(msg);
    }
}

void ChatServer::load_users()
{
    ifs.open(file);
    std::string line;
    while (getline(ifs, line))
    {
        bool b = true;

        std::vector<std::string> results = util::split(line, ", ");
        std::string userid = results[0];
        users.insert({userid, user()});
        users[userid].password = results[1];
        users[userid].isAdmin = stoi(results[2]);
        users[userid].isBanned = stoi(results[3]);
    }
    ifs.close();
}

void ChatServer::save_users()
{
    ofs.open(file);
    for (auto& user: users)
    {
        ofs << user.first << ", "
        << user.second.password << ", "
        << user.second.isAdmin << ", "
        << user.second.isBanned << ", "
        << std::endl;
    }
    ofs.close();
}


