#ifndef ASYNET_TASK_QUEUE_H_
#define ASYNET_TASK_QUEUE_H_

#include <mutex>
#include <queue>
#include "task_handler.h"

namespace asynet {

class TaskQueue {
public:
    TaskQueue() = default;

    task_handler_ptr push(task& t, void* arg) {
        task_handler_ptr ptr(new TaskHandler(t, arg));
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(ptr);
        return ptr;
    }

    size_t size() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    bool empty() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    bool try_pop(task_handler_ptr& res) {
        std::lock_guard<std::mutex> lock(mutex_);
        if(queue_.empty())
            return false;

        res = queue_.front();
        queue_.pop();
        return true;
    }

private:
    std::queue<task_handler_ptr> queue_;
    std::mutex mutex_;
};

}

#endif
