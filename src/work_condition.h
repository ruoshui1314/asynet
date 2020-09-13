

#ifndef ASYNET_WORK_CONDITION_H_
#define ASYNET_WORK_CONDITION_H_

#include <thread>
#include <mutex>
#include <condition_variable>

#include "utils/link_list.h"

namespace asynet {


struct Waiter: public LinkNode<Waiter> {
    std::condition_variable condition;
    bool ready;
    std::mutex mutex;

    Waiter(): ready(false) {}

    void notify_one() {
        ready = true;
        condition.notify_one();
    }

    void wait() {
        std::unique_lock<std::mutex> lk(mutex);
        condition.wait(lk, [this] { return ready; });
    }
};


class Notifier {
public:
    Notifier() {}

    void add_waiter(Waiter* waiter) {
        waiter->ready = false;
        std::lock_guard<std::mutex> lck (mutex_);
        waiters_.append(waiter);
    }

    void notify_waiter(Waiter* waiter) {
        if (waiter->ready)
            return;
        {
            std::lock_guard<std::mutex> lck (mutex_);
            waiter->remove_from_list();
        }
        waiter->notify_one();
    }

    void notify_one() {
        std::lock_guard<std::mutex> lck (mutex_);
        if (waiters_.empty())
            return;
        LinkNode<Waiter>* waiter = waiters_.head();
        waiter->remove_from_list();
        waiter->value()->notify_one();
    }

    void notify_all() {
        std::lock_guard<std::mutex> lck (mutex_);
        if (waiters_.empty())
            return;
        while (!waiters_.empty()) {
            LinkNode<Waiter>* waiter = waiters_.head();
            waiter->remove_from_list();
            waiter->value()->notify_one();
        }
    }

private:
    std::mutex mutex_;
    LinkedList<Waiter> waiters_;
};

}

#endif
