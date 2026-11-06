#include "ChatClient.hpp"

#include <asio.hpp>
#include <deque>
#include <iostream>
#include <queue>
#include <string>

using asio::ip::tcp;

ChatClient::ChatClient(asio::io_context& io) : io_(io), socket_(io), resolver_(io) {}

void ChatClient::connect(const std::string& host, const std::string& port, const std::string& user, const std::string& pass) {
    connecting_ = true;
    last_error_.clear();
    auto endpoints = resolver_.resolve(host, port);
    asio::async_connect(socket_, endpoints,
                        [this, user, pass](std::error_code ec, const tcp::endpoint&) {
                            connecting_ = false;
                            if (!ec) {
                                connected_ = true;
                                do_read();
                                send({"CONNECT|" + user + ", " + pass});
                            } else {
                                last_error_ = ec.message();
                            }
                        });
}

void ChatClient::send(const std::string& msg) {
    bool writing = !write_queue_.empty();
    write_queue_.push_back(msg + '\n');
    if (!writing) do_write();
}

void ChatClient::send_chat_message(const std::string& text) { send({"MSG|" + text}); }

// Pop one received message, if any. Call this each frame.
bool ChatClient::poll_message(std::string& out) {
    if (incoming_.empty()) return false;
    out = std::move(incoming_.front());
    incoming_.pop();
    return true;
}

bool ChatClient::is_connected() const { return connected_; }
bool ChatClient::is_connecting() const { return connecting_; }
const std::string& ChatClient::last_error() const { return last_error_; }

void ChatClient::do_read() {
    asio::async_read_until(socket_, incoming_buf_, '\n',
                           [this](const std::error_code ec, const std::size_t length) {
                               if (ec) return fail(ec);
                               std::string msg{asio::buffers_begin(incoming_buf_.data()), asio::buffers_end(incoming_buf_.data())};
                               if (!msg.empty()) msg.pop_back(); // remove last character: newline
                               incoming_buf_.consume(length);    // clear buffer
                               incoming_.push(msg);
                               do_read();
                           });
}

void ChatClient::do_write() {
    asio::async_write(socket_, asio::buffer(write_queue_.front()),
                      [this](const std::error_code ec, std::size_t) {
                          if (ec) return fail(ec);
                          write_queue_.pop_front();
                          if (!write_queue_.empty()) do_write();
                      });
}

void ChatClient::fail(const std::error_code ec) {
    connected_ = false;
    last_error_ = ec.message();
}
void ChatClient::close()
{
    std::error_code ignore;
    socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ignore);
    socket_.close(ignore);
}
