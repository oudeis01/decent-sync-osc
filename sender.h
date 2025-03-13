#ifndef SENDER_H
#define SENDER_H

#include <lo/lo.h>
#include <string>
#include <queue>
#include "receiver.h"

class Sender {
public:
    void sendAck(const std::string& ip, int port, int index);
    void sendDone(const std::string& ip, int port, int index);
    void sendInfo(const std::string& ip, int port, const std::queue<Command>& queue);
    static void print_connection_info(const std::string& ip, int port);
};

#endif