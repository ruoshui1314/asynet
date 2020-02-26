#ifndef ASYNET_TCP_CONNECTION_H_
#define ASYNET_TCP_CONNECTION_H_

#include "event.h"
#include "socket.h"

#include <memory>
#include <string>

namespace asynet {
class TcpConnection {
public:
    TcpConnection(int fd, const std::string& address, const std::string& port);

private:
    Event event_;
    Socket socket_;
};

using tcp_connection_ptr = std::shared_ptr<TcpConnection>;

}

#endif