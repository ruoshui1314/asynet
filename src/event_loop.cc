#include "event_loop.h"
#include "epoll_reactor.h"

#include <vector>

using namespace asynet;

EventLoop::EventLoop(): running_(true) {
    reactor_ = std::unique_ptr<EpollReactor>(new EpollReactor());
}

bool EventLoop::add_read_event(Event* event_) {
    event_->update_mask(reactor_->get_read_mask());
    return reactor_->add_event(event_);
}

void EventLoop::run() {
    while (running_) {
        std::vector<Event*> events;
        reactor_->poll(events);
    }
}