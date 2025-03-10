#pragma once
#include "oscpp/client.hpp"
#include "oscpp/server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "motor.h"

#define PORT 8000

class OSCReceiver {
public:
    explicit OSCReceiver(MotorController& motorController);
    ~OSCReceiver();

    void start();
    void stop();

private:
    void setupSocket();
    void receiveAndProcessMessage();
    void processMessage(const OSCPP::Server::Message& msg);

    MotorController& motorController_;
    int sockfd_;
    bool running_;
};