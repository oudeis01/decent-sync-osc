#include "receiver.h"
#include "oscpp/server.hpp"
#include <sstream>

#ifdef PI_ZERO
#include <pigpio.h>
#endif

// Constructor
OSCReceiver::OSCReceiver(MotorController& motorController)
    : motorController_(motorController), running_(true), sockfd_(-1) {
    setupSocket();
}

// Destructor
OSCReceiver::~OSCReceiver() {
    stop();
    if (sockfd_ != -1) close(sockfd_);
}

void OSCReceiver::start() {
    std::stringstream ss;
    ss << "OSC Receiver thread: " << std::this_thread::get_id() << '\n'
       << "OSC Receiving on port: " << PORT << '\n'
       << "Format: /motor/rotate <steps> <delay> <direction>\n"
       << "        /disable\n";
    std::cout << ss.str();
    
    while (running_) {
        receiveAndProcessMessage();
    }
}

void OSCReceiver::stop() {
    running_ = false;
    // Close socket to interrupt recvfrom
    if (sockfd_ != -1) {
        shutdown(sockfd_, SHUT_RDWR);
        close(sockfd_);
        sockfd_ = -1;
    }
}

void OSCReceiver::setupSocket() {
    sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_ < 0) {
        throw std::runtime_error("Error opening socket");
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(sockfd_, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        throw std::runtime_error("Error binding socket");
    }
}
void OSCReceiver::processMessage(const OSCPP::Server::Message& msg) {
    const std::string address = msg.address();
    OSCPP::Server::ArgStream args(msg.args());
    
    if (address == "/motor/rotate") {
        if (args.size() >= 3) {  // Use count() instead of remaining()
            MotorController::MotorCommand cmd;
            cmd.steps = args.int32();
            cmd.delay = args.float32();
            cmd.direction = args.int32() > 0;
            motorController_.queueCommand(cmd);
        }
    }
    else if (address == "/disable") {
        std::cout << "Received disable command\n";
        MotorController::MotorCommand cmd;
        cmd.disable = true;
        #ifdef PI_ZERO
        gpioWrite(EN_PIN, 1);
        #endif
        motorController_.queueCommand(cmd);
    }
}

void OSCReceiver::receiveAndProcessMessage() {
    char buffer[1024];
    struct sockaddr_in remote_addr;
    socklen_t remote_len = sizeof(remote_addr);
    
    int n = recvfrom(sockfd_, buffer, sizeof(buffer), 0,
                   (struct sockaddr *)&remote_addr, &remote_len);
    
    if (n < 0) {
        if (running_) std::cerr << "Error in recvfrom\n";
        return;
    }

    try {
        OSCPP::Server::Packet packet(buffer, n);
        if (packet.isBundle()) {
            OSCPP::Server::Bundle bundle(packet);
            OSCPP::Server::PacketStream packets(bundle.packets());
            while (!packets.atEnd()) {
                processMessage(OSCPP::Server::Message(packets.next()));
            }
        } else {
            processMessage(OSCPP::Server::Message(packet));
        }
    } catch (const std::exception& e) {
        std::cerr << "OSC Error: " << e.what() << '\n';
    }
}