#include "tcp_connection.h"
#include "event_loop.h"

#include <unistd.h>

using namespace asynet;

TcpConnection::TcpConnection(EventLoop& loop,
                int fd,
                const std::string& address,
                const std::string& port) :
    loop_(loop),
    event_(fd),
    socket_(address, port, fd) {
    event_.set_close_callback([this]() { this->on_close_callback(); });
    event_.set_read_callback([this]() { this->on_read_callback(); });
    loop_.add_read_event(&event_);
}

void TcpConnection::on_read_callback() {
    int n = ::read(socket_.get_fd(), buffer_, MAX_BUFFER_SIZE);
    if (n > 0) {
        message_ = buffer_;
        if (on_message_cb_)
            on_message_cb_(shared_from_this(), message_);
    } else if (n == 0) {
        on_close_callback();
    }
}

void TcpConnection::on_write_callback() {

}

void TcpConnection::send_message(const std::string& message) {
    int n = ::write(socket_.get_fd(), message.c_str(), message.length());
    if (n < 0)
        on_disconnect_cb_(shared_from_this(), -1);
}

void TcpConnection::on_close_callback() {
    socket_.close();
    if (on_disconnect_cb_)
        on_disconnect_cb_(shared_from_this(), -1);
}