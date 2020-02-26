#include "socket.h"
#include "struct.h"

#include <sys/socket.h>
#include <unistd.h>

using namespace asynet;

int Socket::create_socket_fd(const struct addrinfo& addr) {
    int socket_type = addr.ai_socktype | SOCK_NONBLOCK | SOCK_CLOEXEC;
    fd_ = ::socket(addr.ai_family, socket_type, addr.ai_protocol);
    if (fd_ < 0)
        return fd_;
    if (!set_socket_option(option_))
        close();
    return fd_;
}

bool Socket::set_socket_option(unsigned int option, bool on) {
    option_ |= option;
    int flag = on ? 1 : 0;
    if (option_ & OPT_REUSEABLE_PORT)
        if (setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(flag)) == -1)
            return false;
    return true;
}

bool Socket::listen(const struct sockaddr& addr, int len) {
    if (::bind(fd_, &addr, len) == -1) {
        close();
        return false;
    }
    if (::listen(fd_, SOMAXCONN) == -1) {
        close();
        return false;
    }
    return true;
}

void Socket::close() {
    if (fd_ < 0)
        return;
    ::close(fd_);
    fd_ = -1;
}