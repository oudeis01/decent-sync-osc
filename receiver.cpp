#include "receiver.h"
#include "sender.h"
#include <oscpp/server.hpp>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <iostream>
#include <ifaddrs.h>
#include <sys/socket.h>

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
std::string Receiver::getLocalIp() const {
    std::string local_ip = "0.0.0.0";
    struct ifaddrs *ifaddr, *ifa;
    
    if (getifaddrs(&ifaddr) == -1) {
        return local_ip;
    }

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;
        
        // IPv4 only
        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(ifa->ifa_addr);
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);
            
            // Skip loopback and virtual interfaces
            if (strcmp(ifa->ifa_name, "lo") != 0 && 
                strncmp(ifa->ifa_name, "docker", 6) != 0 &&
                strncmp(ifa->ifa_name, "br-", 3) != 0 &&
                strncmp(ifa->ifa_name, "veth", 4) != 0) {
                local_ip = ip;
                break;
            }
        }
    }

    freeifaddrs(ifaddr);
    return local_ip;

}

void Receiver::start() {
    running_ = true;
    thread_ = std::thread(&Receiver::run, this);
    
    std::cout << "OSC Server started\n";
    std::cout << "Listening on: " << getLocalIp() << ":" << port_ << "\n";
    std::cout << "Supported commands:\n";
    std::cout << "  /rotate <steps> <delay_us> <direction(0|1)>\n";
    std::cout << "  /enable\n  /disable\n  /info\n";
}
void Receiver::stop() {
    running_ = false;
    if (thread_.joinable()) thread_.join();
}

void Receiver::processPacket(const OSCPP::Server::Packet& packet, sockaddr_in& cliaddr) {
    if (packet.isMessage()) {
        OSCPP::Server::Message msg(packet);
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
        std::cout << "New connection from: " << cmd.senderIp << ":" << cmd.senderPort << "\n";
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