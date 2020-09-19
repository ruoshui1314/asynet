#include "tcp_connection.h"
#include "event_loop.h"
#include <iostream>

using namespace asynet;

TcpConnection::TcpConnection(EventLoop& loop, int fd,
                const std::string& address,
                const std::string& port) :
    loop_(loop),
    socket_(address, port, fd),
    connected_(true) {
        event_ = std::unique_ptr<Event>(new Event(fd));
}

void TcpConnection::enable_read() {
    tcp_connection_ptr conn = shared_from_this();
    std::cout << "addr " << &loop_ << std::endl;
    event_->set_close_callback([=](SocketError err) { conn->on_close_callback(err); });
    event_->set_read_callback([=]() {
        std::cout << "callback addr " << &loop_ << std::endl;
        // conn->on_read_callback();
        loop_.add_task(std::bind(&asynet::TcpConnection::on_read_callback, conn));
    });
    event_->update_mask(loop_.get_read_mask());
    loop_.add_event(event_.get());
}

void TcpConnection::enable_write() {
    event_->update_mask(loop_.get_write_mask());
    loop_.add_event(event_.get());
}

void TcpConnection::disable_read() {
    event_->del_mask(loop_.get_read_mask());
    loop_.add_event(event_.get());
}

void TcpConnection::disable_write() {
    event_->del_mask(loop_.get_write_mask());
    loop_.add_event(event_.get());
}

void TcpConnection::on_read_callback() {
    std::cout << "on read callback" << std::endl;
    int n = input_.read_fd(socket_.get_fd());
    if (n > 0) {
        if (on_message_cb_)
            on_message_cb_(shared_from_this(), input_);
    } else if (n == 0) {
        on_close_callback(SocketError::EVENT_EOF);
    } else if (n == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        on_close_callback(SocketError::EVENT_READING);
    }
}

void TcpConnection::send_message(const std::string& message) {
    int n = output_.write_fd(socket_.get_fd(), message);
    if (n <= 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            enable_write();
            return;
        }
        on_close_callback(SocketError::EVENT_WRITING);
        return;
    }
    if (!output_.is_complete())
        loop_.add_task(std::bind(&asynet::TcpConnection::on_write_callback, shared_from_this()));
}

void TcpConnection::on_write_callback() {
    int n = output_.keep_write(socket_.get_fd());
    if (n > 0) {
        disable_write();
        return;
    }

    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        enable_write();
    }
    on_close_callback(SocketError::EVENT_WRITING);
}

void TcpConnection::close() {
    socket_.close();
    connected_ = false;
    event_->reset_mask();
    loop_.del_event(event_.get());
    event_.reset();
    input_.clear();
    output_.clear();
}

void TcpConnection::on_close_callback(SocketError err) {
    connected_ = false;
    socket_.close();
    event_->reset_mask();
    loop_.del_event(event_.get());
    event_.reset();
    if (on_disconnect_cb_)
        on_disconnect_cb_(shared_from_this(), err);
    input_.clear();
    output_.clear();
}
