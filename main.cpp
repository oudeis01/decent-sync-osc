#include "main.h"

int main() {
    if (gpioInitialise() < 0) return 1;

    std::queue<Command> cmd_queue;
    std::mutex queue_mutex;
    std::condition_variable cv;
    std::atomic<int> cmd_index(0);

    Motor motor(EN_PIN, DIR_PIN, STEP_PIN);
    Receiver receiver(9000, cmd_queue, queue_mutex, cmd_index, cv);
    receiver.start();

    while (true) {
        std::unique_lock<std::mutex> lock(queue_mutex);
        cv.wait(lock, [&cmd_queue]{ return !cmd_queue.empty(); });

        Command cmd = cmd_queue.front();
        cmd_queue.pop();
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