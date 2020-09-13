#include "task_control.h"

using namespace asynet;

static thread_local TaskQueue* local_work_queue;
static thread_local unsigned local_index;

TaskControl::TaskControl(int thread_num):selector_(0) {
    if (thread_num <= 0)
        thread_num = 1;
    for (int i = 0; i < thread_num; ++i) {
        work_queues_.push_back(std::unique_ptr<TaskQueue>(new TaskQueue));
        work_threads_.push_back(std::thread(&TaskControl::run_work_thread, this, i));
    }
}

task_handler_ptr TaskControl::add_task(task& t, void* arg = nullptr) {
    task_handler_ptr ptr = work_queues_[selector_]->push(t, arg);
    selector_ = (selector_ + 1) % work_queues_.size();
}

bool TaskControl::pop_local_queue(task_handler_ptr& ptr) {
    if (!local_work_queue)
        return false;
    return local_work_queue->try_pop(ptr);
}

bool TaskControl::steal_other_thread_queue(task_handler_ptr& ptr) {
    for(int i = 0; i < work_queues_.size(); ++i) {
        unsigned const index=(local_index+i+1) % work_queues_.size();
        if(work_queues_[index]->try_steal(ptr))
            return true;
    }
    return false;
}

void TaskControl::run_work_thread(int index) {
    local_index = index;
    local_work_queue = work_queues_[index].get();
    while (running_) {
        task_handler_ptr task;
        if(pop_local_queue(task) || steal_other_thread_queue(task)) {
            task->run();
        } else {
            std::this_thread::yield();
        }
    }
}

TaskControl::~TaskControl() {
    running_ = false;
    for (auto& t : work_threads_)
        t.join();
}
