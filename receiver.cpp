#include "receiver.h"
#include "sender.h"
#include "colorPalette.h"
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>

Receiver::Receiver(int port, std::queue<Command>& queue, std::mutex& mutex,
                   std::atomic<int>& cmdIndex, std::condition_variable& cv)
    : port_(port), commandQueue_(queue), queueMutex_(mutex),
      commandIndex_(cmdIndex), cv_(cv), server_thread_(nullptr) {}

Receiver::~Receiver() {
    stop(false);
}

void Receiver::start() {
    server_thread_ = lo_server_thread_new(std::to_string(port_).c_str(), nullptr);
    lo_server_thread_add_method(server_thread_, nullptr, nullptr, 
                               &Receiver::oscHandler, this);
    lo_server_thread_start(server_thread_);
    
    std::cout << Color::successTag() << " OSC Server started\n"
              << Color::cmdTag() << " Listening on: " << Color::value(getLocalIp()) 
              << ":" << Color::value(port_) << "\n"
              << Color::cmdTag() << " Supported commands:\n"
              << Color::cmdTag() << "   /rotate <steps> <delay_us> <direction(0|1)>\n"
              << Color::cmdTag() << "   /enable\n   /disable\n   /info\n   /exit\n";
}

void Receiver::stop(bool wait_for_thread) {
    if (server_thread_) {
        lo_server_thread_stop(server_thread_);
        if (wait_for_thread) {
            lo_server_thread_free(server_thread_);
            server_thread_ = nullptr;
        }
    }
}

// Rest of receiver.cpp remains the same as in previous correct version