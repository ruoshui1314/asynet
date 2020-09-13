#include "event_loop.h"
#include "epoll_reactor.h"

#include <vector>
#include <algorithm>

using namespace asynet;

EventLoop::EventLoop(int thread_num): running_(true),
    timer_event_(*this),
    control_(thread_num) {
    reactor_ = std::unique_ptr<EpollReactor>(new EpollReactor());
    reactor_->init_reactor();
    timer_event_.create_timer_fd();
}

bool EventLoop::add_event(Event* event) {
    return reactor_->add_event(event);
}

bool EventLoop::del_event(Event* event) {
    return reactor_->del_event(event);
}

void EventLoop::add_timer(Timer* t, int index) {
    timer_event_.add_timer(t, index);
}

void EventLoop::del_timer(Timer* t) {
    timer_event_.del_timer(t);
}

void EventLoop::add_task(task t, void* arg) {
    control_.add_task(t, arg);
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