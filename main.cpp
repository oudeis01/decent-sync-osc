#include "main.h"

int main() {
#ifdef ARCH
    std::cout << "Running on x86_64 architecture\n";
#elif defined(PI_ZERO)
    std::cout << "Running on ARM architecture\n";
#endif
    try {
        MotorController motorController;
        motorController.start();

        OSCReceiver oscReceiver(motorController);
        oscReceiver.start();

        // TODO: Add a signal handler to stop the threads gracefully
        motorController.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
