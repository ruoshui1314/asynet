#include "tcp_connection.h"
#include "event_loop.h"

using namespace asynet;

TcpConnection::TcpConnection(int fd,
                const std::string& address,
                const std::string& port) :
    socket_(address, port, fd) {
        event_ = std::unique_ptr<Event>(new Event(fd));
}

void TcpConnection::enable_read(EventLoop& loop) {
    tcp_connection_ptr conn = shared_from_this();
    event_->set_close_callback([=](SocketError err) { conn->on_close_callback(conn, err); });
    event_->set_read_callback([=]() { conn->on_read_callback(conn); });
    loop.add_read_event(event_.get());
}

void TcpConnection::on_read_callback(const tcp_connection_ptr& conn) {
    int n = input_.read_socket(socket_.get_fd());
    if (n > 0) {
        if (on_message_cb_)
            on_message_cb_(shared_from_this(), input_);
    } else if (n == 0) {
        on_close_callback(conn, SocketError::EVENT_READING);
    } else if (n == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        on_close_callback(conn, SocketError::EVENT_READING);
    }
}

void TcpConnection::on_write_callback(const tcp_connection_ptr& conn) {
    if (!output_.has_data())
        return;
    int n = output_.write_buffer_cache(socket_.get_fd());
    if (n == 0)
        on_close_callback(conn, SocketError::EVENT_EOF);
}

void TcpConnection::send_message(std::string&& message) {
    int n = output_.write_socket(socket_.get_fd(), std::forward<std::string>(message));
    if (n == 0) {
        on_close_callback(shared_from_this(), SocketError::EVENT_EOF);
    }
    if (n == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        on_close_callback(shared_from_this(), SocketError::EVENT_WRITING);
    }
}

void TcpConnection::on_close_callback(const tcp_connection_ptr& conn, SocketError err) {
    socket_.close();
    if (on_disconnect_cb_)
        on_disconnect_cb_(shared_from_this(), err);
    event_.reset();
}
