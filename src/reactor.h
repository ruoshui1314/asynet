#ifndef ASYNET_REACTOR_H_
#define ASYNET_REACTOR_H_

#include "event.h"

#include <vector>

namespace asynet {
class Reactor {
public:
    virtual bool init_reactor() = 0;
    virtual bool add_event(Event* event) = 0;
    virtual bool del_event(Event* event) = 0;
    virtual int get_read_mask() = 0;
    virtual void poll(std::vector<Event*>& events) = 0;
};

}

#endif