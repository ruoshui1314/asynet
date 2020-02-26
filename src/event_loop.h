#ifndef ASYNET_EVENT_LOOP_H_
#define ASYNET_EVENT_LOOP_H_


#include "event.h"
#include "reactor.h"

#include <memory>

namespace asynet {
class EventLoop {
public:
    EventLoop();

    bool add_read_event(Event* event_);
    void run();

private:
    std::unique_ptr<Reactor> reactor_;
    bool running_;

};

}

#endif