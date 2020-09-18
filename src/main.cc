#include "tcp_server.h"
#include "event_loop.h"

#include <iostream>
#include <list>
#include <buffer.h>

#include <algorithm>
#include <queue>

#include "timer.h"

using namespace std;
using namespace asynet;

void on_connect(const tcp_connection_ptr& conn) {
    cout << conn->get_address() << ":" << conn->get_port() << " connected." << endl;
}

void on_message(const tcp_connection_ptr& conn, ReadBuffer& buffer) {
    std::string message = buffer.read_all();
    cout << "on message:" << message << endl;
    conn->send_message(std::move(message));
}

void on_disconnect(const tcp_connection_ptr& conn, SocketError error) {
    cout << "on disconnect:" << static_cast<int>(error) << endl;
}

void on_timer() {
    cout << "on time " << Timer::now() << endl;
}

int main() {
    EventLoop loop;
    std::string addr = "0.0.0.0";
    std::string port = "10000";
    TcpServer s(loop, addr, port);
    s.set_connect_callback(on_connect);
    s.set_on_message_callback(on_message);
    s.set_disconnect_callback(on_disconnect);
    Timer t(loop);
    cout << "now " << Timer::now() << endl;
    t.run_every(5, on_timer);
    s.start();
    loop.run();
    return 0;
}