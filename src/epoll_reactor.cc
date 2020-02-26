#include "epoll_reactor.h"

#include <sys/epoll.h>
#include <cstring>

using namespace asynet;

EpollReactor::EpollReactor() {
}

bool EpollReactor::init_reactor() {
    epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
    return epoll_fd_ >= 0;
}

bool EpollReactor::add_event(Event* event) {
    struct epoll_event ee;
    ::bzero(&ee, sizeof(ee));
    EVENT_OPEATION operation = event->get_operation();
    int op = operation == EVENT_OPEATION::INIT ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    ee.data.fd = event->get_fd();
    ee.data.ptr = event;
    return epoll_ctl(epoll_fd_, op, ee.data.fd, &ee) != -1;
}

bool EpollReactor::del_event(Event* event) {

}

int EpollReactor::get_read_mask() {
    return EPOLLIN | EPOLLPRI;
}

void EpollReactor::poll(std::vector<Event*>& events) {
    int res = epoll_wait(epoll_fd_, events_, MAX_EVENTS, -1);
    if (res < 0)
        return;
    for (int i = 0; i < res; ++i) {
        int what = events_[i].events;
    }
}