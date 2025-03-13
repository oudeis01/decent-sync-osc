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

        std::cout << "\nExecuting command #" << cmd.index 
                  << " from " << cmd.senderIp 
                  << ":" << cmd.senderPort << "\n";

        try {
            switch (cmd.type) {
                case Command::ROTATE:
                    std::cout << "Starting rotation - Steps: " << cmd.steps
                            << ", Delay: " << cmd.delayUs << "μs\n";
                    motor.rotate(cmd.steps, static_cast<int>(cmd.delayUs), cmd.direction);
                    break;
                case Command::ENABLE:
                    std::cout << "Enabling motor\n";
                    motor.enable();
                    break;
                case Command::DISABLE:
                    std::cout << "Disabling motor\n";
                    motor.disable();
                    break;
                case Command::INFO:
                    continue;  // Already handled in receiver
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error executing command: " << e.what() << "\n";
        }

        Sender sender;
        sender.sendDone(cmd.senderIp, 12345, cmd.index);
        std::cout << "Completed command #" << cmd.index << " for "
          << cmd.senderIp << ":12345\n";
    }

    receiver.stop();
    gpioTerminate();
    return 0;
}