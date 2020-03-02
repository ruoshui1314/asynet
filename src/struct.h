#ifndef ASYNET_STRUCT_H_
#define ASYNET_STRUCT_H_

#include <memory>

namespace asynet {

const unsigned int OPT_REUSEABLE_PORT = 1;

class TcpConnection;
class Buffer;

using tcp_connection_ptr = std::shared_ptr<TcpConnection>;
using on_connect_callback = std::function<void(const tcp_connection_ptr&)>;
using on_message_callback = std::function<void(const tcp_connection_ptr&, Buffer&)>;
using on_disconnect_callback = std::function<void(const tcp_connection_ptr&, int error)>;

}

#endif