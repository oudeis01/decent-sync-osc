#include "main.h"
#include "colorPalette.h"

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

        std::cout << "\n" << Color::cmdTag() << " Executing command #" << Color::value(cmd.index) 
                  << " from " << Color::client(cmd.senderIp) 
                  << ":" << Color::value(cmd.senderPort) << "\n";

        try {
            switch (cmd.type) {
                case Command::ROTATE:
                    std::cout << Color::runTag() << " Starting rotation - Steps: " << Color::value(cmd.steps)
                            << ", Delay: " << Color::value(cmd.delayUs) << "μs\n";
                    motor.rotate(cmd.steps, static_cast<int>(cmd.delayUs), cmd.direction);
                    Sender::sendDone(cmd.senderIp, cmd.senderPort, cmd.index);
                    break;
                case Command::ENABLE:
                    std::cout << Color::runTag() << " Enabling motor\n";
                    motor.enable();
                    Sender::sendDone(cmd.senderIp, cmd.senderPort, cmd.index);
                    break;
                case Command::DISABLE:
                    std::cout << Color::runTag() << " Disabling motor\n";
                    motor.disable();
                    Sender::sendDone(cmd.senderIp, cmd.senderPort, cmd.index);
                    break;
                case Command::INFO:
                    continue;  // Already handled in receiver
            }
        } catch (const std::exception& e) {
            std::cerr << Color::errorTag() << " Error executing command: " << e.what() << "\n";
        }
    }

    gpioTerminate();
    return 0;
}