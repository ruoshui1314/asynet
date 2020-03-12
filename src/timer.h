#ifndef ASYNET_TIMER_H_
#define ASYNET_TIMER_H_

#include "event.h"

#include <cstdint>
#include <functional>
#include <vector>
#include <memory>

namespace asynet {
const int MICROSECOND_PER_SECOND = 1000 * 1000;

class EventLoop;

class TimerHeap;

class TimerEvent;

using timer_callback = std::function<void()>;

class Timer {
public:
    Timer(EventLoop& loop);
    
    void run_after(double delay_seconds, timer_callback cb);

    void run_every(double delay_seconds, timer_callback cb);

    void run_at(uint64_t time, timer_callback cb);

    void cancel();

    static uint64_t now();

private:
    void reset();
    bool is_repeat() { return interval_ > 0.0; }
    void restart_timestamp();

    EventLoop& loop_;
    uint64_t timestamp_;
    double interval_;
    int index_;
    timer_callback cb_;
    friend class TimerHeap;
    friend class TimerEvent;
};

class TimerHeap {
public:
    void push_heap(Timer* t);

    void pop_heap();

    void remove(Timer* t);

    void update(Timer* t);

    Timer* top() { return heap_.front(); }

    bool empty() { return heap_.empty(); }

private:
    void up_heap(Timer* t);

    void down_heap(Timer* t);
    
    std::vector<Timer*> heap_;

    void swap(size_t index1, size_t index2);
};

class TimerEvent {
public:
    TimerEvent(EventLoop& loop);
    bool create_timer_fd();
    void handle_read_callback();

    void add_timer(Timer* t, int index);

    void del_timer(Timer* t);

private:
    void update_time();
    EventLoop& loop_;
    int timer_fd_;
    std::unique_ptr<Event> event_;
    TimerHeap heap_;
};

}

#endif