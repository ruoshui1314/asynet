#include "timer.h"
#include "event_loop.h"

#include <sys/time.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <cstring>

using namespace asynet;

Timer::Timer(EventLoop& loop): loop_(loop), interval_(0.0), index_(-1) {}

void Timer::run_after(double delay_seconds, timer_callback cb) {
    interval_ = 0.0;
    timestamp_ = now() + delay_seconds * MICROSECOND_PER_SECOND;
    cb_ = std::move(cb);
    loop_.add_timer(this, index_);
}

void Timer::restart_timestamp() {
    timestamp_ = now() + interval_ * MICROSECOND_PER_SECOND;
}

uint64_t Timer::now() {
    struct timeval tv;
    ::gettimeofday(&tv, NULL);
    return tv.tv_sec * MICROSECOND_PER_SECOND + tv.tv_usec;
}

void Timer::run_every(double delay_seconds, timer_callback cb) {
    interval_ = delay_seconds;
    run_after(delay_seconds, cb);
}

void Timer::run_at(uint64_t time, timer_callback cb) {
    interval_ = 0.0;
    timestamp_ = time;
    cb_ = std::move(cb);
    loop_.add_timer(this, index_);
}

void Timer::reset() {
    cb_ = nullptr;
    timestamp_ = 0;
    index_ = -1;
}

void Timer::cancel() {
    loop_.del_timer(this);
}

void TimerHeap::push_heap(Timer* t) {
    t->index_ = heap_.size();
    heap_.push_back(t);
    up_heap(t);
}

void TimerHeap::up_heap(Timer* t) {
    while (t->index_ > 0) {
        size_t parent = (t->index_ - 1) / 2;
        Timer* time = heap_[parent];
        if (time->timestamp_ <= t->timestamp_)
            break;
        swap(t->index_, parent);
        t = time;
    }
}

void TimerHeap::pop_heap() {
    if (heap_.size() <= 1) {
        heap_.pop_back();
        return;
    }
    swap(0, heap_.size() - 1);
    heap_.pop_back();
    down_heap(heap_[0]);
}

void TimerHeap::swap(size_t index1, size_t index2) {
    Timer* tmp = heap_[index1];
    heap_[index1] = heap_[index2];
    heap_[index2] = tmp;
    heap_[index1]->index_ = index1;
    heap_[index2]->index_ = index2;
}

void TimerHeap::down_heap(Timer* t) {
    size_t child = 2 * t->index_ + 1;
    while (child < heap_.size()) {
        if (child + 1 < heap_.size() && heap_[child]->timestamp_ > heap_[child+1]->timestamp_)
            child += 1;
        if (heap_[t->index_]->timestamp_ <= heap_[child]->timestamp_)
            break;
        swap(t->index_, child);
        t = heap_[child];
        child = 2 * child + 1;
    }
}

void TimerHeap::remove(Timer* t) {
    size_t index = t->index_;
    if (heap_.size() <= index || heap_[index] != t) {
        t->index_ = -1;
        return;
    }
    swap(index, heap_.size() - 1);
    t = heap_[index];
    heap_.pop_back();
    if (t->index_ > 0 && heap_[(t->index_-1)/2]->timestamp_ > t->timestamp_)
        up_heap(t);
    else
        down_heap(t);
}

void TimerHeap::update(Timer* t) {
    size_t index = t->index_;
    if (heap_.size() <= index || heap_[index] != t)
        return;
    if (t->index_ > 0 && heap_[(t->index_-1)/2]->timestamp_ > t->timestamp_)
        up_heap(t);
    else
        down_heap(t);
}


TimerEvent::TimerEvent(EventLoop& loop): loop_(loop) {
}

bool TimerEvent::create_timer_fd() {
    timer_fd_ = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timer_fd_ < 0)
        return false;
    event_ = std::unique_ptr<Event>(new Event(timer_fd_));
    event_->update_mask(loop_.get_read_mask());
    event_->set_read_callback([this](){this->handle_read_callback();});
    loop_.add_event(event_.get());
    return true;
}

void TimerEvent::handle_read_callback() {
    uint64_t time;
    ssize_t n = ::read(timer_fd_, &time, sizeof(time));
    uint64_t now = Timer::now();
    std::vector<Timer*> timers;
    while (!heap_.empty()) {
        Timer* timer = heap_.top();
        if (timer->timestamp_ > now)
            break;
        heap_.pop_heap();
        timers.push_back(timer);
    }
    for (auto& t : timers) {
        if (t->cb_)
            t->cb_();
        if (t->is_repeat()) {
            t->restart_timestamp();
            heap_.push_heap(t);
        }
    }
    update_time();
}

void TimerEvent::update_time() {
    struct itimerspec time_spec;
    bzero(&time_spec, sizeof(time_spec));
    if (heap_.empty()) {
        int ret = ::timerfd_settime(timer_fd_, 0, &time_spec, nullptr);
        return;
    }
    Timer* time = heap_.top();
    int64_t diff = time->timestamp_ - Timer::now();
    if (diff <= 0) {
        time_spec.it_value.tv_nsec = 100 * 1000;
    } else {
        time_spec.it_value.tv_sec = diff / MICROSECOND_PER_SECOND;
        time_spec.it_value.tv_nsec = diff % MICROSECOND_PER_SECOND * 1000;
    }
    int ret = ::timerfd_settime(timer_fd_, 0, &time_spec, nullptr);
}


void TimerEvent::add_timer(Timer* t, int index) {
    if (index < 0)
        heap_.push_heap(t);
    else
        heap_.update(t);
    if (t->index_ == 0)
        update_time();
}

void TimerEvent::del_timer(Timer* t) {
    heap_.remove(t);
}