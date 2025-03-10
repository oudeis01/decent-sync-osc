#pragma once
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "glob.h"
// #if defined(PI_ZERO)
    #include <pigpio.h>
    #define EN_PIN 24
    #define DIR_PIN 23
    #define STEP_PIN 18
// #endif
class MotorController {
public:
    struct MotorCommand {
        int steps;
        float delay;
        bool direction;
        bool disable;
    };

    MotorController() : running_(true) {}
    void start();
    void stop();
    void queueCommand(const MotorCommand& cmd);

private:
    void moveMotor(int steps, float delay, bool direction, bool disable);
    void motorControlThread();

    std::queue<MotorCommand> commandQueue_;
    std::mutex queueMutex_;
    std::condition_variable queueCV_;
    std::thread motorThread_;
    bool running_;
};