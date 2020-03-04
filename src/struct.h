#ifndef ASYNET_STRUCT_H_
#define ASYNET_STRUCT_H_

#include <memory>

namespace asynet {

const unsigned int OPT_REUSEABLE_PORT = 1;

class TcpConnection;
class Buffer;

enum class SocketError {
    EVENT_READING = 1, /**< error encountered while reading */
    EVENT_WRITING = 2, /**< error encountered while writing */
    EVENT_EOF     = 3, /**< eof file reached */
    EVENT_ERROR   = 4, /**< unrecoverable error encountered */
    EVENT_TIMEOUT = 5, /**< user-specified timeout reached */
    EVENT_CONNECTED = 6 /**< connect operation finished. */
};

using tcp_connection_ptr = std::shared_ptr<TcpConnection>;
using on_connect_callback = std::function<void(const tcp_connection_ptr&)>;
using on_message_callback = std::function<void(const tcp_connection_ptr&, Buffer&)>;
using on_disconnect_callback = std::function<void(const tcp_connection_ptr&, SocketError error)>;

}

#endif