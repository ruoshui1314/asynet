#include "acceptor.h"

#include <netdb.h>
#include <cstring>
#include <arpa/inet.h>

using namespace asynet;

Acceptor::Acceptor(const std::string& address,
                const std::string& port,
                unsigned int option): socket_(address, port) {
    socket_.set_option(option);
}

bool Acceptor::listen() {
    struct addrinfo hints, *servinfo, *p;
    int result;
    int fd;
    bzero(&hints, sizeof(hints));
    std::string address = socket_.get_address();
    std::string port = socket_.get_port();
    if (address.find(':') != std::string::npos)
        hints.ai_family = AF_INET6;
    else
        hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((result = ::getaddrinfo(address.c_str(), port.c_str(), &hints, &servinfo)) != 0)
        return false;
    for (p = servinfo; p != nullptr; p = p->ai_next) {
        fd = socket_.create_socket_fd(*p);
        if (fd == -1)
            continue;
        if (!socket_.listen(*(p->ai_addr), p->ai_addrlen))
            return false;
        break;
    }
    event_ = std::unique_ptr<Event>(new Event(fd));
    event_->set_read_callback([this](){ this->handle_read_callback(); });
    ::freeaddrinfo(servinfo);
    return true;
}

bool Acceptor::enable_read(EventLoop& loop) {
    return loop.add_read_event(event_.get());
}

void Acceptor::set_on_connect_callback(on_connect_callback& cb) {
    on_connect_cb_ = std::move(cb);
}

void Acceptor::handle_read_callback() {
    struct sockaddr_storage sa;
    socklen_t sa_len = sizeof(sa);
    int fd = ::accept4(socket_.get_fd(),
                        (struct sockaddr*)& sa,
                        &sa_len,
                        SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (fd < 0)
        return;
    char ip[46];
    int port;
    if (sa.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)&sa;
        ::inet_ntop(AF_INET, (void*)&(s->sin_addr), ip, sizeof(ip));
        port = ntohs(s->sin_port);
    } else {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&sa;
        ::inet_ntop(AF_INET6, (void*)&(s->sin6_addr), ip, sizeof(ip));
        port = ntohs(s->sin6_port);
    }
    std::string addr(ip);
    std::string port_s(std::to_string(port));
    tcp_connection_ptr connection =
        std::make_shared<TcpConnection>(fd, addr, port_s);
    connections_[fd] = connection;
    if (on_connect_cb_)
        on_connect_cb_(connection);
}