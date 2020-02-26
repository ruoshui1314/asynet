#ifndef ASYNET_EPOLL_REACTOR_H_
#define ASYNET_EPOLL_REACTOR_H_


#include "reactor.h"

#include <sys/epoll.h>

namespace asynet {

const int MAX_EVENTS = 32;

class EpollReactor: public Reactor {
public:
    EpollReactor();

    bool init_reactor() override;
    bool add_event(Event* event) override;
    bool del_event(Event* event) override;
    int get_read_mask() override;
    void poll(std::vector<Event*>& events) override;

private:
    int epoll_fd_;
    struct epoll_event events_[MAX_EVENTS];
};

}

#endif