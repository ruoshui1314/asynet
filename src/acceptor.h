#ifndef ASYNET_ACCEPTOR_H_
#define ASYNET_ACCEPTOR_H_

#include "struct.h"
#include "socket.h"
#include "event.h"
#include "event_loop.h"
#include "tcp_connection.h"

#include <string>
#include <memory>
#include <map>

namespace asynet {

class Acceptor {
public:
    explicit Acceptor(EventLoop& loop,
                    const std::string& address,
                    const std::string& port,
                    unsigned int option = OPT_REUSEABLE_PORT);

    bool listen();

    bool enable_read();

    void handle_read_callback();

    void set_on_connect_callback(on_connect_callback& cb);

    void set_on_message_callback(on_message_callback& cb);

    void set_on_disconnect_callback(on_disconnect_callback& cb);
private:
    EventLoop& loop_;
    Socket socket_;
    std::unique_ptr<Event> event_;
    on_connect_callback on_connect_cb_;
    on_message_callback on_message_cb_;
    on_disconnect_callback on_disconnect_cb_;
};

}

#endif