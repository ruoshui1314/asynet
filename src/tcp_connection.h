#ifndef ASYNET_TCP_CONNECTION_H_
#define ASYNET_TCP_CONNECTION_H_

#include "event.h"
#include "socket.h"
#include "struct.h"
#include "buffer.h"

#include <memory>
#include <string>

namespace asynet {

class EventLoop;

class TcpConnection: public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop& loop, int fd,
                const std::string& address,
                const std::string& port);
    std::string get_address() { return socket_.get_address(); }
    std::string get_port() { return socket_.get_port(); }

    void set_on_message_callback(const on_message_callback& cb) {
        on_message_cb_ = cb;
    }

    void set_on_disconnect_callback(const on_disconnect_callback& cb) {
        on_disconnect_cb_ = cb;
    }

    void on_read_callback();

    void on_write_callback();

    void on_close_callback(SocketError err);

    void send_message(const std::string& message);

    void enable_read();

    void enable_write();

    void disable_read();

    void disable_write();

    void keep_write();

    void close();

private:
    EventLoop& loop_;
    std::unique_ptr<Event> event_;
    Socket socket_;
    on_message_callback on_message_cb_;
    on_disconnect_callback on_disconnect_cb_;
    ReadBuffer input_;
    WriteBuffer output_;
    bool connected_;
};

}

#endif