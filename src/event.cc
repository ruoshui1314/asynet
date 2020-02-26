#include "event.h"

using namespace asynet;

Event::Event(int fd) :
    fd_(fd),
    mask_(0),
    operation_(EVENT_OPEATION::INIT)
    {}
