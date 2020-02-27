#include "tcp_server.h"

using namespace asynet;

bool TcpServer::start() {
    if (!acceptor_.listen())
        return false;
    return acceptor_.enable_read();
}

void TcpServer::set_connect_callback(on_connect_callback cb) {
    acceptor_.set_on_connect_callback(cb);
}

void TcpServer::set_on_message_callback(on_message_callback cb) {
    acceptor_.set_on_message_callback(cb);
}

void TcpServer::set_disconnect_callback(on_disconnect_callback cb) {
    acceptor_.set_on_disconnect_callback(cb);
}