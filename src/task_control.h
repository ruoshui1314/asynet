#ifndef ASYNET_TASK_CONTROL_H_
#define ASYNET_TASK_CONTROL_H_

#include <vector>
#include <thread>
#include <memory>

#include "task_queue.h"
#include "struct.h"
#include "work_thread.h"
#include "work_condition.h"

namespace asynet {

class TaskControl {
public:
    TaskControl(int thread_num): selector_(0) {
        if (thread_num <= 0)
            thread_num = 1;
        work_threads_.resize(thread_num);
        waiters_.resize(thread_num);
        for (int i = 0; i < thread_num; ++i) {
            waiters_[i] = std::unique_ptr<Waiter>(new Waiter());
            work_threads_[i] = std::unique_ptr<WorkThread>(new WorkThread(this, i, waiters_[i].get()));
        }
    }

    ~TaskControl() {
        notify.notify_all();
        for (auto& w : work_threads_)
            w->stop();
    }

    task_handler_ptr add_task(task& t, void* arg = nullptr) {
        task_handler_ptr ptr = work_threads_[selector_]->push_task(t, arg);
        selector_ = (selector_ + 1) % work_threads_.size();
        return ptr;
    }

    void notify_waiter(Waiter* waiter) {
        notify.notify_waiter(waiter);
    }

    void notify_one() {
        notify.notify_one();
    }

    void commit_waiter(Waiter* waiter) {
        notify.add_waiter(waiter);
    }

    bool pop_other_thread_queue(int index, task_handler_ptr& ptr) {
        for(size_t i = 0; i < work_threads_.size(); ++i) {
            index = (index + i + 1) % work_threads_.size();
            if(work_threads_[index]->steal_task(ptr)) {
                return true;
            }
        }
        return false;
    }

private:
    std::vector<std::unique_ptr<WorkThread>> work_threads_;
    int selector_;
    Notifier notify;
    std::vector<std::unique_ptr<Waiter>> waiters_;
};

}

#endif
