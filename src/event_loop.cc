#include "event_loop.h"
#include "epoll_reactor.h"

#include <vector>

using namespace asynet;

EventLoop::EventLoop(): running_(true) {
    reactor_ = std::unique_ptr<EpollReactor>(new EpollReactor());
    reactor_->init_reactor();
}

bool EventLoop::add_event(Event* event) {
    return reactor_->add_event(event);
}

bool EventLoop::del_event(Event* event) {
    return reactor_->del_event(event);
}

void EventLoop::run() {
    while (running_) {
        std::vector<Event*> events;
        reactor_->poll(events);
        for (auto& event : events) {
            event->handle_event();
        }
    }
}