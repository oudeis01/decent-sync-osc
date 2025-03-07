#include "motor.h"

void MotorController::start() {
    motorThread_ = std::thread(&MotorController::motorControlThread, this);
#ifdef PI_ZERO
    wiringPiSetupGpio();
    pinMode(EN_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(STEP_PIN, OUTPUT);
    pullUpDnControl(EN_PIN, PUD_UP);
    digitalWrite(EN_PIN, LOW);
#endif

}

void MotorController::stop() {
    running_ = false;
    queueCV_.notify_one();
    if (motorThread_.joinable()) {
        motorThread_.join();
    }
#ifdef PI_ZERO
    digitalWrite(EN_PIN, HIGH);
#endif
}

void MotorController::queueCommand(const MotorCommand& cmd) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    commandQueue_.push(cmd);
    queueCV_.notify_one();
}

void MotorController::moveMotor(int steps, float delay, bool direction) {
#ifdef ARCH
    // Placeholder for motor control logic
    std::cout << "Moving motor: \n" 
                "\tsteps: "<< steps << "\n" <<
                "\tdelay: "<< delay << "ms\n" <<
                "\tdirec: " << (direction ? "CW" : "CCW") << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(int(delay)*steps));
#elif defined(PI_ZERO)
    std::cout << "Moving motor: \n" 
                "\tsteps: "<< steps << "\n" <<
                "\tdelay: "<< delay << "ms\n" <<
                "\tdirec: " << (direction ? "CW" : "CCW") << "\n";
    digitalWrite(DIR_PIN, direction);
    for(int i=0; i<steps; ++i){
        digitalWrite(STEP_PIN, HIGH);
        std::this_thread::sleep_for(std::chrono::microseconds(
            static_cast<int>(delay)));
        digitalWrite(STEP_PIN, LOW);
        std::this_thread::sleep_for(std::chrono::microseconds(
            static_cast<int>(delay)));
    }
#endif
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