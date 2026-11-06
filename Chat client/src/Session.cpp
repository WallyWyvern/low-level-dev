#include "Session.hpp"

#include <asio.hpp>
#include <deque>
#include <iostream>
#include <memory>
#include <string>

using asio::ip::tcp;

Session::Session(tcp::socket socket, MessageHandler on_message, CloseHandler on_close)
    : socket_(std::move(socket)),
      on_message_(std::move(on_message)),
      on_close_(std::move(on_close)) {}

void Session::start() { do_read(); }

void Session::send(const std::string& message) {
    asio::post(socket_.get_executor(), [this, message]() mutable {
        const bool writing = !write_queue_.empty();
        write_queue_.push_back(message + '\n');
        if (!writing) do_write();
    });
}

const std::string& Session::id() const { return id_; }
void Session::set_id(std::string id) { id_ = std::move(id); }

void Session::set_nickname(const std::string& new_nick) { this->nickname_ = new_nick; }
const std::string& Session::nickname() const { return this->nickname_; }

void Session::do_read() {
    auto self = shared_from_this();
    asio::async_read_until(socket_, incoming_buf_, '\n',
                           [this, self](const std::error_code ec, const std::size_t size) {
                               if (ec) return close_with_error(ec);

                               std::string data{asio::buffers_begin(incoming_buf_.data()), asio::buffers_end(incoming_buf_.data())};
                               if (!data.empty()) data.pop_back(); // remove last character: newline
                               incoming_buf_.consume(size);        // clear buffer

                               on_message_(self, data);
                               do_read();
                           });
}

void Session::do_write() {
    asio::async_write(socket_, asio::buffer(write_queue_.front()),
                      [this](const std::error_code ec, std::size_t) {
                          if (ec) return close_with_error(ec);
                          write_queue_.pop_front();
                          if (!write_queue_.empty()) do_write();
                      });
}

void Session::close_with_error(const std::error_code ec) {
    if (ec != asio::error::eof) {
        std::cerr << "Session " << id_ << " error: " << ec.message() << "\n";
    }
    std::error_code ignore;
    socket_.close(ignore);
    on_close_(shared_from_this());
}

void Session::close()
{
    std::error_code ignore;
    socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ignore);
    socket_.close(ignore);
    on_close_(shared_from_this());
}
