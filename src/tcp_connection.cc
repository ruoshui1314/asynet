#include "tcp_connection.h"

using namespace asynet;

TcpConnection::TcpConnection(int fd,
                        const std::string& address,
                        const std::string& port) :
    event_(fd),
    socket_(address, port, fd) {
}
