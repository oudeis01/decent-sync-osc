#ifndef SENDER_H
#define SENDER_H

#include <string>
#include <queue>
#include "receiver.h"

class Sender {
public:
    void sendAck(const std::string& ip, int port, int index);
    void sendDone(const std::string& ip, int port, int index);
    void sendInfo(const std::string& ip, int port, const std::queue<Command>& queue);
};

#endif