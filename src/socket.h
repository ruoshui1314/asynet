#ifndef ASYNET_SOCKET_H_
#define ASYNET_SOCKET_H_


#include <string>
#include <netdb.h>

namespace asynet{

class Socket {
public:
    explicit Socket(const std::string& address,
        const std::string& port, int fd = -1): fd_(fd),
        address_(address),
        port_(port),
        option_(0) {}

    std::string get_address() {
        return address_;
    }

    std::string get_port() {
        return port_;
    }

    void set_option(unsigned int option) {
        option_ = option;
    }

    bool listen(const struct sockaddr& addr, int len);

    void close();

    int create_socket_fd(const struct addrinfo& addr);

    int get_fd() { return fd_; }

    bool set_socket_option(unsigned int option, bool on = true);

    int get_bytes_readable_on_socket();

private:
    int fd_;
    const std::string address_;
    const std::string port_;
    unsigned int option_;
};
}

#endif