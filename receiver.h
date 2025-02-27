#pragma once
#include <osc++.hpp>
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
    OSCReceiver(MotorController& motorController) 
        : motorController_(motorController), running_(true) {
        setupSocket();
    }

    ~OSCReceiver() {
        stop();
        close(sockfd_);
    }

    void start();
    void stop();

private:
    void setupSocket();
    void receiveAndProcessMessage();
    void processMessage(const osc::message& msg);

    MotorController& motorController_;
    int sockfd_;
    bool running_;
};