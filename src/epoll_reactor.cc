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
    ee.events = event->get_mask();
    int op = operation == EVENT_OPEATION::INIT ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    ee.data.ptr = event;
    return epoll_ctl(epoll_fd_, op, event->get_fd(), &ee) != -1;
}

bool EpollReactor::del_event(Event* event) {
    struct epoll_event ee;
    ::bzero(&ee, sizeof(ee));
    ee.data.ptr = event;
    ee.events = event->get_mask();
    int op = EPOLL_CTL_DEL;
    return epoll_ctl(epoll_fd_, op, event->get_fd(), &ee) != -1;
}

int EpollReactor::get_read_mask() {
    return EPOLLIN | EPOLLPRI | EPOLLET;
}

void EpollReactor::poll(std::vector<Event*>& events) {
    int res = epoll_wait(epoll_fd_, events_, MAX_EVENTS, -1);
    if (res < 0)
        return;
    for (int i = 0; i < res; ++i) {
        int what = events_[i].events;
        Event* event = static_cast<Event*>(events_[i].data.ptr);
        EVENT_STATE state = EVENT_STATE::INIT;
        if (what & (EPOLLIN | EPOLLHUP))
            state = EVENT_STATE::READ;
        else if (what & EPOLLOUT)
            state = EVENT_STATE::WRITE;
        else if (what & (EPOLLERR | EPOLLRDHUP))
            state = EVENT_STATE::CLOSED;
        event->set_event_result_state(state);
        events.push_back(event);
    }
}