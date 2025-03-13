#include "sender.h"
#include "colorPalette.h"
#include <lo/lo.h>
#include <iostream>

constexpr int RESPONSE_PORT = 12345;

void Sender::sendAck(const std::string& ip, int port, int index) {
    lo_address addr = lo_address_new(ip.c_str(), std::to_string(port).c_str());
    lo_message msg = lo_message_new();
    lo_message_add_int32(msg, index);
    lo_send_message(addr, "/ack", msg);
    lo_message_free(msg);
    lo_address_free(addr);
}

void Sender::sendDone(const std::string& ip, int port, int index) {
    lo_address addr = lo_address_new(ip.c_str(), std::to_string(port).c_str());
    lo_message msg = lo_message_new();
    lo_message_add_int32(msg, index);
    lo_send_message(addr, "/done", msg);
    lo_message_free(msg);
    lo_address_free(addr);
    std::cout << Color::successTag() << " Completed command #" << Color::value(index) 
              << " for " << Color::client(ip) << ":" << Color::value(port) << "\n";
}

void Sender::sendInfo(const std::string& ip, int port, const std::queue<Command>& queue) {
    lo_address addr = lo_address_new(ip.c_str(), std::to_string(RESPONSE_PORT).c_str());
    lo_message msg = lo_message_new();

    if (queue.empty()) {
        lo_message_add_string(msg, "no commands in the queue");
        lo_send_message(addr, "/info", msg);
    } else {
        std::queue<Command> q_copy = queue;
        while (!q_copy.empty()) {
            const Command& cmd = q_copy.front();
            switch (cmd.type) {
                case Command::ROTATE:
                    lo_message_add_string(msg, "rotate");
                    lo_message_add_int32(msg, cmd.steps);
                    lo_message_add_float(msg, cmd.delayUs);  // Send as float
                    lo_message_add_int32(msg, cmd.direction);
                    break;
                case Command::ENABLE:
                    lo_message_add_int32(msg, cmd.index);
                    lo_message_add_string(msg, "enable");
                    break;
                case Command::DISABLE:
                    lo_message_add_int32(msg, cmd.index);
                    lo_message_add_string(msg, "disable");
                    break;
                case Command::INFO:
                    lo_message_add_int32(msg, cmd.index);
                    lo_message_add_string(msg, "info");
                    break;
            }
            q_copy.pop();
        }
        lo_send_message(addr, "/info", msg);
    }
    lo_message_free(msg);
    lo_address_free(addr);
}