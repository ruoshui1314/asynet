#include "work_thread.h"
#include "task_control.h"

using namespace asynet;

WorkThread::WorkThread(TaskControl* control, int index, Waiter* waiter):
    control_(control),
    running_(true),
    index_(index),
    waiter_(waiter) {
    thread_ = std::thread(&WorkThread::run, this);
}

task_handler_ptr WorkThread::push_task(task& t, void* arg) {
    task_handler_ptr ptr = queue_.push(t, arg);
    control_->notify_waiter(waiter_);
    if (queue_.size() > 1)
        control_->notify_one();
    return ptr;
}

bool WorkThread::steal_task(task_handler_ptr& ptr){
    if (queue_.size() <= 1)
        return false;
    return queue_.try_pop(ptr);
}

void WorkThread::stop() {
    running_ = false;
    thread_.join();
}

void WorkThread::run() {
    while (running_) {
        waiter_->wait();
        task_handler_ptr task;
        if(queue_.try_pop(task) || control_->pop_other_thread_queue(index_, task)) {
            task->run();
        } else {
            control_->commit_waiter(waiter_);
        }
    }
}