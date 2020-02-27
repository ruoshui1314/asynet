#include "tcp_server.h"
#include "event_loop.h"

#include <iostream>

using namespace std;
using namespace asynet;

void on_connect(const tcp_connection_ptr& conn) {
    cout << conn->get_address() << ":" << conn->get_port() << " connected." << endl;
}

void on_message(const tcp_connection_ptr& conn, const std::string& message) {
    cout << "on message:" << message << endl;
    conn->send_message(message);
}

void on_disconnect(const tcp_connection_ptr& conn, int error) {
    cout << "on disconnect:" << error << endl;
}

int main() {
    EventLoop loop;
    std::string addr = "0.0.0.0";
    std::string port = "10000";
    TcpServer s(loop, addr, port);
    s.set_connect_callback(on_connect);
    s.set_on_message_callback(on_message);
    s.set_disconnect_callback(on_disconnect);
    s.start();
    loop.run();
    return 0;
}