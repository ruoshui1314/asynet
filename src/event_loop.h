#ifndef ASYNET_EVENT_LOOP_H_
#define ASYNET_EVENT_LOOP_H_


#include "event.h"
#include "reactor.h"
#include "timer.h"
#include "task_control.h"

#include <memory>

namespace asynet {
class EventLoop {
public:
    EventLoop(int thread_num = std::thread::hardware_concurrency());

    bool add_event(Event* event);

    bool del_event(Event* event);

    void run();

    int get_read_mask() { return reactor_->get_read_mask(); }

    void add_timer(Timer* t, int index);

    void del_timer(Timer* t);

    void add_task(task t, void* arg = nullptr);

private:
    std::unique_ptr<Reactor> reactor_;
    TaskControl control_;
    bool running_;
    TimerEvent timer_event_;
};

}

#endif