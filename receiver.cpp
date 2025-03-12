#include "receiver.h"
#include "sender.h"
#include <oscpp/server.hpp>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <iostream>

Receiver::Receiver(int port, std::queue<Command>& queue, std::mutex& mutex,
                   std::atomic<int>& cmdIndex, std::condition_variable& cv)
    : port_(port), commandQueue_(queue), queueMutex_(mutex),
      commandIndex_(cmdIndex), cv_(cv), running_(false) {
    sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port_);
    bind(sockfd_, (sockaddr*)&servaddr, sizeof(servaddr));
}

Receiver::~Receiver() {
    stop();
    close(sockfd_);
}

void Receiver::start() {
    running_ = true;
    thread_ = std::thread(&Receiver::run, this);
}

void Receiver::stop() {
    running_ = false;
    if (thread_.joinable()) thread_.join();
}

void Receiver::processPacket(const OSCPP::Server::Packet& packet, sockaddr_in& cliaddr) {
    if (packet.isMessage()) {
        OSCPP::Server::Message msg = packet.getMessage();
        std::string address = msg.address();

        Command cmd{};
        cmd.senderIp = inet_ntoa(cliaddr.sin_addr);
        cmd.senderPort = ntohs(cliaddr.sin_port);

        if (address == "/rotate") {
            OSCPP::Server::ArgStream args = msg.args();
            cmd.type = Command::ROTATE;
            cmd.steps = args.int32();
            cmd.delayUs = args.int32();
            cmd.direction = args.int32();
            cmd.index = ++commandIndex_;

            std::lock_guard<std::mutex> lock(queueMutex_);
            commandQueue_.push(cmd);
            cv_.notify_one();
        } else if (address == "/enable") {
            cmd.type = Command::ENABLE;
            cmd.index = ++commandIndex_;
            std::lock_guard<std::mutex> lock(queueMutex_);
            commandQueue_.push(cmd);
            cv_.notify_one();
        } else if (address == "/disable") {
            cmd.type = Command::DISABLE;
            cmd.index = ++commandIndex_;
            std::lock_guard<std::mutex> lock(queueMutex_);
            commandQueue_.push(cmd);
            cv_.notify_one();
        } else if (address == "/info") {
            Sender sender;
            std::lock_guard<std::mutex> lock(queueMutex_);
            sender.sendInfo(cmd.senderIp, cmd.senderPort, commandQueue_);
            return;
        }

        Sender sender;
        sender.sendAck(cmd.senderIp, cmd.senderPort, cmd.index);
    }
}

void Receiver::run() {
    sockaddr_in cliaddr{};
    socklen_t len = sizeof(cliaddr);
    char buffer[8192];

    while (running_) {
        ssize_t n = recvfrom(sockfd_, buffer, sizeof(buffer), 0, (sockaddr*)&cliaddr, &len);
        if (n > 0) {
            try {
                OSCPP::Server::Packet packet(buffer, n);
                if (packet.isBundle()) {
                    OSCPP::Server::Bundle bundle(packet);
                    OSCPP::Server::PacketStream packets(bundle.packets());
                    while (!packets.atEnd()) {
                        processPacket(packets.next(), cliaddr);
                    }
                } else {
                    processPacket(packet, cliaddr);
                }
            } catch (const std::exception& e) {
                std::cerr << "OSC Error: " << e.what() << std::endl;
            }
        }
    }
}