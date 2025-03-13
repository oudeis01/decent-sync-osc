#include "main.h"
#include "colorPalette.h"


void commandWorker(Motor& motor, 
                  std::queue<Command>& cmd_queue,
                  std::mutex& queue_mutex,
                  std::condition_variable& cv) {
    while (worker_running) {
        std::unique_lock<std::mutex> lock(queue_mutex);
        cv.wait(lock, [&]{ return !cmd_queue.empty() || !worker_running; });
        
        if (!worker_running) break;
        
        Command cmd = cmd_queue.front();
        cmd_queue.pop();
        lock.unlock();

        try {
            switch (cmd.type) {
                case Command::ROTATE:
                    std::cout << Color::cmdTag() << " Starting rotation - Steps: " 
                            << Color::value(cmd.steps) << ", Delay: " 
                            << Color::value(cmd.delayUs) << "μs\n";
                    motor.rotate(cmd.steps, static_cast<int>(cmd.delayUs), cmd.direction);
                    Sender::sendDone(cmd.senderIp, 12345, cmd.index);
                    break;
                // ... other cases ...
                case Command::EXIT:
                    worker_running = false;
                    break;
            }
        } catch (const std::exception& e) {
            std::cerr << Color::errorTag() << " Error executing command: " << e.what() << "\n";
        }
    }
}

std::atomic<bool> shutdown_flag(false);

void signalHandler(int signal) {
    std::cerr << Color::errorTag() << " Received signal " << signal 
              << ", initiating shutdown..." << std::endl;
    shutdown_flag.store(true);
}


int main() {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    std::signal(SIGSEGV, signalHandler);

    if (gpioInitialise() < 0) return 1;

    std::queue<Command> cmd_queue;
    std::mutex queue_mutex;
    std::condition_variable cv;
    std::atomic<int> cmd_index(0);

    Motor motor(EN_PIN, DIR_PIN, STEP_PIN);
    Receiver receiver(9000, cmd_queue, queue_mutex, cmd_index, cv);
    receiver.start();

    std::thread worker(commandWorker, 
                    std::ref(motor), 
                    std::ref(cmd_queue),
                    std::ref(queue_mutex), 
                    std::ref(cv));

    while (!shutdown_flag) {
        std::unique_lock<std::mutex> lock(queue_mutex);
        cv.wait_for(lock, std::chrono::milliseconds(100), 
            [&]{ return !cmd_queue.empty() || shutdown_flag; });

        if (shutdown_flag) break;

        if (!cmd_queue.empty()) {
            Command cmd = cmd_queue.front();
            cmd_queue.pop();
            lock.unlock();
            std::unique_lock<std::mutex> lock(queue_mutex);
            cv.wait(lock, [&cmd_queue]{ return !cmd_queue.empty(); });


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
                    case Command::EXIT:
                        std::cout << Color::successTag() 
                                << " Graceful shutdown initiated via OSC\n";
                        shutdown_flag.store(true);
                        break;
                }
            } catch (const std::exception& e) {
                std::cerr << Color::errorTag() << " Error executing command: " << e.what() << "\n";
            }
        }
    receiver.stop();
    motor.disable();
    gpioTerminate();
    std::cout << Color::successTag() << " Application shutdown complete\n";
    return 0;
    }
}