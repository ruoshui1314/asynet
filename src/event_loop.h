#ifndef ASYNET_EVENT_LOOP_H_
#define ASYNET_EVENT_LOOP_H_


#include "event.h"
#include "reactor.h"

#include <memory>

namespace asynet {
class EventLoop {
public:
    EventLoop();

    bool add_event(Event* event);
    bool del_event(Event* event);
    void run();
    int get_read_mask() { return reactor_->get_read_mask(); }
private:
    std::unique_ptr<Reactor> reactor_;
    bool running_;

};

}

#endif