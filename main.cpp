#include "motor.h"
#include "receiver.h"
#include <pigpio.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

#define EN_PIN 24
#define DIR_PIN 23
#define STEP_PIN 18

int main() {
    if (gpioInitialise() < 0) return 1;

    std::queue<Command> cmdQueue;
    std::mutex queueMutex;
    std::condition_variable cv;
    std::atomic<int> cmdIndex(0);

    Motor motor(EN_PIN, DIR_PIN, STEP_PIN);
    Receiver receiver(9000, cmdQueue, queueMutex, cmdIndex, cv);
    receiver.start();

    while (true) {
        std::unique_lock<std::mutex> lock(queueMutex);
        cv.wait(lock, [&cmdQueue]{ return !cmdQueue.empty(); });

        Command cmd = cmdQueue.front();
        cmdQueue.pop();
        lock.unlock();

        switch (cmd.type) {
            case Command::ROTATE:
                motor.rotate(cmd.steps, cmd.delayUs, cmd.direction);
                break;
            case Command::ENABLE:
                motor.enable();
                break;
            case Command::DISABLE:
                motor.disable();
                break;
        }

        Sender sender;
        sender.sendDone(cmd.senderIp, cmd.senderPort, cmd.index);
    }

    receiver.stop();
    gpioTerminate();
    return 0;
}