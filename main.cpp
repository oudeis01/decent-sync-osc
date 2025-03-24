#include "main.h"
#include "colorPalette.h"
#include <thread>
#include <chrono>

std::atomic<bool> shutdown_flag(false);
std::atomic<bool> worker_running(true);

void commandWorker(Motor& motor, 
                  std::queue<Command>& cmd_queue,
                  std::mutex& queue_mutex,
                  std::condition_variable& cv) {
    while (worker_running && !shutdown_flag) {
        std::unique_lock<std::mutex> lock(queue_mutex);
        cv.wait(lock, [&]{ 
            return !cmd_queue.empty() || !worker_running || shutdown_flag; 
        });

        if (!worker_running || shutdown_flag) break;
        
        Command cmd = cmd_queue.front();
        cmd_queue.pop();
        lock.unlock();

        try {
            switch (cmd.type) {
                case Command::ROTATE:
                    std::cout << Color::runTag() << " Starting rotation - Steps: " 
                            << Color::value(cmd.steps) << ", Delay: " 
                            << Color::value(cmd.delayUs) << "μs\n";
                    motor.rotate(cmd.steps, static_cast<int>(cmd.delayUs), cmd.direction);
                    Sender::sendDone(cmd.senderIp, 12345, cmd.index, std::string("ROTATE"));
                    break;
                case Command::ENABLE:
                    std::cout << Color::runTag() << " Enabling motor\n";
                    motor.enable();
                    Sender::sendDone(cmd.senderIp, 12345, cmd.index, std::string("ENABLE"));
                    break;
                case Command::DISABLE:
                    std::cout << Color::runTag() << " Disabling motor\n";
                    motor.disable();
                    Sender::sendDone(cmd.senderIp, 12345, cmd.index, std::string("DISABLE"));
                    break;
                case Command::EXIT:
                    std::cout << Color::successTag() << " Graceful shutdown initiated via OSC\n";
                    shutdown_flag.store(true);  // Add this line
                    worker_running = false;
                    break;
                case Command::INFO:
                    break;
            }
        } catch (const std::exception& e) {
            std::cerr << Color::errorTag() << " Error executing command: " << e.what() << "\n";
        }
    }
}

void signalHandler(int signal) {
    if (!shutdown_flag.exchange(true)) {
        std::cerr << Color::errorTag() << " Received signal " << signal 
                << ", initiating shutdown..." << std::endl;
    }
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

    // Main monitoring loop
    while (!shutdown_flag) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Cleanup sequence
    worker_running = false;
    shutdown_flag = true;
    cv.notify_one();

    if (worker.joinable()) {
        worker.join();
    }

    receiver.stop(true);  // Ensure waiting for server thread
    motor.disable();
    gpioTerminate();

    std::cout << Color::successTag() << " Application shutdown complete\n";
    return 0;
}