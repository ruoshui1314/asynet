#include "event.h"

using namespace asynet;

Event::Event(int fd) :
    fd_(fd),
    mask_(0),
    operation_(EVENT_OPEATION::INIT)
    {}

void Event::handle_event() {
    if (state_ == EVENT_STATE::READ && read_cb_)
        read_cb_();
    else if (state_ == EVENT_STATE::WRITE && write_cb_)
        write_cb_();
    else if (state_ == EVENT_STATE::CLOSED && close_cb_)
        close_cb_(SocketError::EVENT_ERROR);
}
