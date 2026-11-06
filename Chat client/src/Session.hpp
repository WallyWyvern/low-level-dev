#pragma once

#include <asio.hpp>
#include <deque>
#include <memory>
#include <string>

class Session;
using SessionPtr = std::shared_ptr<Session>;

using asio::ip::tcp;

// Session: a connected client, owns the socket
class Session : public std::enable_shared_from_this<Session> {
public:
    using MessageHandler = std::function<void(SessionPtr, std::string)>;
    using CloseHandler = std::function<void(SessionPtr)>;

    Session(tcp::socket socket, MessageHandler on_message, CloseHandler on_close);

    void start();
    void close();
    void send(const std::string& message);

    const std::string& id() const;
    void set_id(std::string id);
    void set_nickname(const std::string& new_nick);
    const std::string& nickname() const;

private:
    void do_read();
    void do_write();

    void close_with_error(std::error_code ec);

    tcp::socket socket_;

    MessageHandler on_message_;
    CloseHandler on_close_;

    std::string id_;
    std::string nickname_;

    asio::streambuf incoming_buf_;
    std::deque<std::string> write_queue_;
};
