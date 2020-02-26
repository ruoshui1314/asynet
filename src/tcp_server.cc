#include "tcp_server.h"

using namespace asynet;

bool TcpServer::start() {
    if (!acceptor_.listen())
        return false;
    return acceptor_.enable_read(loop_);
}

void TcpServer::set_connect_callback(on_connect_callback cb) {
    
}