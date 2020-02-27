#ifndef ASYNET_TCP_SERVER_H_
#define ASYNET_TCP_SERVER_H_


#include "event_loop.h"
#include "acceptor.h"

#include <string>
#include <map>


namespace asynet {

class TcpServer {
public:
    explicit TcpServer(EventLoop& loop,
                    const std::string& address,
                    const std::string& port,
                    unsigned int option = 0):
        acceptor_(loop, address, port, option)
    {}

    bool start();

    void set_connect_callback(on_connect_callback cb);

    void set_on_message_callback(on_message_callback cb);

    void set_disconnect_callback(on_disconnect_callback cb);

private:
    Acceptor acceptor_;
    std::map<int, tcp_connection_ptr> connections;

};

}

#endif