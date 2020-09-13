#ifndef ASYNET_WORK_THREAD_H_
#define ASYNET_WORK_THREAD_H_

#include "task_queue.h"
#include "struct.h"
#include "work_condition.h"

#include <thread>

namespace asynet {

class TaskControl;

class WorkThread {
public:
    WorkThread(TaskControl* control, int index, Waiter* waiter);

    task_handler_ptr push_task(task& t, void* arg);

    bool steal_task(task_handler_ptr& ptr);

    void stop();

private:
    void run();

private:
    TaskControl* control_;
    bool running_;
    int index_;
    Waiter* waiter_;
    TaskQueue queue_;
    std::thread thread_;
};

}

#endif
