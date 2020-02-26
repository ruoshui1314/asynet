#ifndef ASYNET_ACCEPTOR_H_
#define ASYNET_ACCEPTOR_H_


#include "socket.h"
#include "event.h"
#include "event_loop.h"
#include "tcp_connection.h"

#include <string>
#include <memory>
#include <map>

namespace asynet {

using on_connect_callback = std::function<void(tcp_connection_ptr)>;

class Acceptor {
public:
    explicit Acceptor(const std::string& address,
                    const std::string& port,
                    unsigned int option);

    bool listen();

    bool enable_read(EventLoop& loop);

    void handle_read_callback();

    void set_on_connect_callback(on_connect_callback& cb);
private:
    Socket socket_;
    std::unique_ptr<Event> event_;
    on_connect_callback on_connect_cb_;
    std::map<int, tcp_connection_ptr> connections_;
};

}

#endif