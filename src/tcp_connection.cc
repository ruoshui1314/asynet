#include "tcp_connection.h"
#include "event_loop.h"

#include <unistd.h>

using namespace asynet;

TcpConnection::TcpConnection(int fd,
                const std::string& address,
                const std::string& port) :
    socket_(address, port, fd) {
        event_ = std::unique_ptr<Event>(new Event(fd));
}

void TcpConnection::enable_read(EventLoop& loop) {
    tcp_connection_ptr conn = shared_from_this();
    event_->set_close_callback([=]() { conn->on_close_callback(conn); });
    event_->set_read_callback([=]() { conn->on_read_callback(conn); });
    loop.add_read_event(event_.get());
}

void TcpConnection::on_read_callback(const tcp_connection_ptr& conn) {
    int n = ::read(socket_.get_fd(), buffer_, MAX_BUFFER_SIZE);
    if (n > 0) {
        message_ = buffer_;
        if (on_message_cb_)
            on_message_cb_(shared_from_this(), message_);
    } else if (n == 0) {
        on_close_callback(conn);
    }
}

void TcpConnection::on_write_callback(const tcp_connection_ptr& conn) {

}

void TcpConnection::send_message(const std::string& message) {
    int n = ::write(socket_.get_fd(), message.c_str(), message.length());
    if (n < 0)
        on_disconnect_cb_(shared_from_this(), -1);
}

void TcpConnection::on_close_callback(const tcp_connection_ptr& conn) {
    socket_.close();
    if (on_disconnect_cb_)
        on_disconnect_cb_(shared_from_this(), -1);
    event_.reset();
}