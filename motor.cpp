#include "motor.h"

void MotorController::start() {
    motorThread_ = std::thread(&MotorController::motorControlThread, this);
}

void MotorController::stop() {
    running_ = false;
    queueCV_.notify_one();
    if (motorThread_.joinable()) {
        motorThread_.join();
    }
}

void MotorController::queueCommand(const MotorCommand& cmd) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    commandQueue_.push(cmd);
    queueCV_.notify_one();
}

void MotorController::moveMotor(int steps, int delay, bool direction) {
    // Placeholder for motor control logic
    std::cout << "Moving motor: \n" 
                "\tsteps: "<< steps << "\n" <<
                "\tdelay: "<< delay << "ms\n" <<
                "\tdirec: " << (direction ? "CW" : "CCW") << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(delay*steps));
}

void MotorController::motorControlThread() {
    while (running_) {
        std::unique_lock<std::mutex> lock(queueMutex_);
        queueCV_.wait(lock, [this] { return !commandQueue_.empty() || !running_; });
        
        if (!running_) break;
        
        MotorCommand cmd = commandQueue_.front();
        commandQueue_.pop();
        lock.unlock();
        
        moveMotor(cmd.steps, cmd.delay, cmd.direction);
    }
}