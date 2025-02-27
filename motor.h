#pragma once
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

class MotorController {
public:
    struct MotorCommand {
        int steps;
        int delay;
        bool direction;
    };

    MotorController() : running_(true) {}
    void start();
    void stop();
    void queueCommand(const MotorCommand& cmd);

private:
    void moveMotor(int steps, int delay, bool direction);
    void motorControlThread();

    std::queue<MotorCommand> commandQueue_;
    std::mutex queueMutex_;
    std::condition_variable queueCV_;
    std::thread motorThread_;
    bool running_;
};