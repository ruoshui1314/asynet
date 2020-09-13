#ifndef ASYNET_TASK_HANDLER_H_
#define ASYNET_TASK_HANDLER_H_

#include <functional>
#include <memory>
#include <atomic>

namespace asynet {

typedef std::function<void(void*)> task;

enum class TaskStatus {
    INIT,
    CANCELED,
    DONE
};

class TaskHandler {
public:
    TaskHandler(task& t, void* arg) {
        t_ = std::move(t);
        arg_ = arg;
        status_.store(TaskStatus::INIT, std::memory_order_relaxed);
    }

    bool cancel() {
        if (swap(TaskStatus::CANCELED))
            return true;
        return false;
    }

private:
    bool swap(const TaskStatus& status) {
        TaskStatus init = TaskStatus::INIT;
        if (status_.compare_exchange_strong(init, status, std::memory_order_relaxed))
            return true;
        return false;
    }

    void run() {
        if (swap(TaskStatus::DONE))
            t_(arg_);
    }

    friend class WorkThread;
    task t_;
    void* arg_;
    std::atomic<TaskStatus> status_;
};

using task_handler_ptr = std::shared_ptr<TaskHandler>;

}

#endif
