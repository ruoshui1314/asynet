#ifndef ASYNET_EVENT_H_
#define ASYNET_EVENT_H_

#include "struct.h"

#include <functional>

namespace asynet {

enum class EVENT_OPEATION {
    INIT = 0,
    ADD = 1,
    DEL = 2
};


typedef std::function<void(void)> callback;

class Event {
public:
    Event(int fd);
    void set_read_callback(callback cb) { read_cb_ = std::move(cb); }
    void set_write_callback(callback cb) { write_cb_ = std::move(cb); }
    void set_close_callback(callback cb) { close_cb_ = std::move(cb); }
    void set_error_callback(callback cb) { error_cb_ = std::move(cb); }

    void handle_event();

    EVENT_OPEATION get_operation() { return operation_; }
    void set_operation(const EVENT_OPEATION& op) { operation_ = op; }
    int get_mask() { return mask_; }
    void update_mask(int mask) { mask_ |= mask; }
    int get_fd() { return fd_; }

private:
    int fd_;
    int mask_;
    EVENT_OPEATION operation_;
    callback read_cb_;
    callback write_cb_;
    callback close_cb_;
    callback error_cb_;
};

}

#endif