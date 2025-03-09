#include "receiver.h"

void OSCReceiver::start() {
    std::stringstream ss;
    ss << "OSC Receiver thread: " << std::this_thread::get_id() << '\n';
    ss << "OSC Receiving on: " << inet_ntoa(((struct sockaddr_in) { .sin_addr = { INADDR_ANY } }).sin_addr) << ":" << PORT << '\n';
    ss << "Format: /motor/rotate <steps> <delay> <direction>\n";
    std::cout << ss.str();
    while (running_) {
        receiveAndProcessMessage();
    }
}

void OSCReceiver::stop() {
    running_ = false;
}

void OSCReceiver::setupSocket() {
        sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd_ < 0) {
            throw std::runtime_error("Error opening socket");
        }

        struct sockaddr_in local_addr;
        memset(&local_addr, 0, sizeof(local_addr));
        local_addr.sin_family = AF_INET;
        local_addr.sin_addr.s_addr = INADDR_ANY;
        local_addr.sin_port = htons(PORT);

        if (bind(sockfd_, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
            throw std::runtime_error("Error on binding");
        }
    }

void OSCReceiver::receiveAndProcessMessage() {
    char buffer[1024];
    struct sockaddr_in remote_addr;
    socklen_t remote_len = sizeof(remote_addr);
    int n = recvfrom(sockfd_, buffer, sizeof(buffer), 0, (struct sockaddr *)&remote_addr, &remote_len);
    
    if (n < 0) {
        std::cerr << "Error in recvfrom" << std::endl;
        return;
    }

    try {
        osc::packet packet(buffer, n);
        auto element = packet.parse();
        if (element.is_message()) {
            processMessage(element.to_message());
        }
    } catch (std::invalid_argument& e) {
        std::cerr << "Error parsing OSC packet: " << e.what() << std::endl;
    }
}

void OSCReceiver::processMessage(const osc::message& msg) {
    if (msg.address() == "/motor/rotate" && msg.values().size() == 3) {
        MotorController::MotorCommand cmd;
        cmd.steps = static_cast<int>(msg.value(0).to_float());
        cmd.delay = msg.value(1).to_float();
        cmd.direction = msg.value(2).to_int32() > 0;
        
        motorController_.queueCommand(cmd);
    }
    if(msg.address() == "/disable") {
        MotorController::MotorCommand cmd;
        cmd.disable = true;
        motorController_.queueCommand(cmd);
    }
}