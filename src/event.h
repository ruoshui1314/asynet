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
typedef std::function<void(SocketError)> close_callback;

enum class EVENT_STATE {
    INIT = 0,
    READ = 1,
    WRITE = 2,
    CLOSED = 3
};

class Event {
public:
    Event(int fd);
    void set_read_callback(callback cb) { read_cb_ = std::move(cb); }
    void set_write_callback(callback cb) { write_cb_ = std::move(cb); }
    void set_close_callback(close_callback cb) { close_cb_ = std::move(cb); }

    void handle_event();

    void set_event_result_state(EVENT_STATE state) { state_ = state; }

    EVENT_OPEATION get_operation() { return operation_; }
    void set_operation(const EVENT_OPEATION& op) { operation_ = op; }
    void update_mask(int mask) { mask_ |= mask; }
    void del_mask(int mask) { mask_ &= ~mask; }
    void reset_mask() { mask_ = 0; }
    int get_mask() { return mask_; }
    int get_fd() { return fd_; }

private:
    int fd_;
    int mask_;
    EVENT_STATE state_;
    EVENT_OPEATION operation_;
    callback read_cb_;
    callback write_cb_;
    close_callback close_cb_;
};

}

#endif