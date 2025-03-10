#include "motor.h"

void MotorController::start() {
    motorThread_ = std::thread(&MotorController::motorControlThread, this);
#ifdef PI_ZERO
    if (gpioInitialise() < 0) {
        throw std::runtime_error("Failed to initialize pigpio");
    }
    gpioSetMode(EN_PIN, PI_OUTPUT);
    gpioSetMode(DIR_PIN, PI_OUTPUT);
    gpioSetMode(STEP_PIN, PI_OUTPUT);
    gpioSetPullUpDown(EN_PIN, PI_PUD_UP);
    gpioWrite(EN_PIN, 0); // Enable LOW
#endif
}

void MotorController::stop() {
    running_ = false;
    queueCV_.notify_one();
    if (motorThread_.joinable()) {
        motorThread_.join();
    }
#ifdef PI_ZERO
    gpioWrite(EN_PIN, 1); // Disable HIGH
    gpioTerminate();
#endif
}
void MotorController::queueCommand(const MotorCommand& cmd) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    commandQueue_.push(cmd);
    queueCV_.notify_one();
}

void MotorController::moveMotor(int steps, float delay, bool direction, bool disable) {
#ifdef ARCH
    // Placeholder for motor control logic
    std::cout << "Moving motor: \n" 
                "\tsteps: "<< steps << "\n" <<
                "\tdelay: "<< delay << "ms\n" <<
                "\tdirec: " << (direction ? "CW" : "CCW") << "\n" <<
                "\tdisable: " << (disable ? "true" : "false") << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(int(delay)*steps));
#elif defined(PI_ZERO)
    std::cout << "Moving motor: \n" 
                "\tsteps: "<< steps << "\n" <<
                "\tdelay: "<< delay << "ms\n" <<
                "\tdirec: " << (direction ? "CW" : "CCW") << "\n" <<
                "\tdisable: " << (disable ? "true" : "false") << "\n";
    if(disable){
        gpioWrite(EN_PIN, 1);
        return;
    }
    gpioWrite(DIR_PIN, direction);
    for(int i=0; i<steps; ++i){
        gpioWrite(STEP_PIN, 1);
        std::this_thread::sleep_for(std::chrono::microseconds(
            static_cast<int>(delay)));
        gpioWrite(STEP_PIN, 0);
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
        
        moveMotor(cmd.steps, cmd.delay, cmd.direction, cmd.disable);
    }
}