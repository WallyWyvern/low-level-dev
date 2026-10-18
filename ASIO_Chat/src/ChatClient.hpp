#pragma once

#include <asio.hpp>
#include <deque>
#include <queue>
#include <string>

using asio::ip::tcp;

class ChatClient {
public:
    explicit ChatClient(asio::io_context& io);

    void connect(const std::string& host, const std::string& port);
    void send(const std::string& msg);
    void send_chat_message(const std::string& text);

    bool poll_message(std::string& out);

    [[nodiscard]] bool is_connected() const;
    [[nodiscard]] bool is_connecting() const;
    [[nodiscard]] const std::string& last_error() const;

private:
    void do_read();
    void do_write();

    void fail(std::error_code ec);

    asio::io_context& io_;
    tcp::socket socket_;
    tcp::resolver resolver_;
    bool connected_ = false;
    bool connecting_ = false;
    std::string last_error_;

    asio::streambuf incoming_buf_;
    std::deque<std::string> write_queue_;
    std::queue<std::string> incoming_;
};
