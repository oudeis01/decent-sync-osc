#include "receiver.h"
#include "sender.h"
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>

Receiver::Receiver(int port, std::queue<Command>& queue, std::mutex& mutex,
                   std::atomic<int>& cmdIndex, std::condition_variable& cv)
    : port_(port), commandQueue_(queue), queueMutex_(mutex),
      commandIndex_(cmdIndex), cv_(cv) {
    server_thread_ = lo_server_thread_new(std::to_string(port_).c_str(), nullptr);
    lo_server_thread_add_method(server_thread_, nullptr, nullptr, 
                               &Receiver::oscHandler, this);
}

Receiver::~Receiver() {
    stop();
}

std::string Receiver::getLocalIp() const {
    std::string local_ip = "0.0.0.0";
    struct ifaddrs *ifaddr, *ifa;
    
    if (getifaddrs(&ifaddr) == -1) return local_ip;

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;
        if (ifa->ifa_addr->sa_family == AF_INET) {
            auto* addr = reinterpret_cast<sockaddr_in*>(ifa->ifa_addr);
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);
            
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
    lo_server_thread_start(server_thread_);
    std::cout << "OSC Server started\n"
              << "Listening on: " << getLocalIp() << ":" << port_ << "\n"
              << "Supported commands:\n"
              << "  /rotate <steps> <delay_us> <direction(0|1)>\n"
              << "  /enable\n  /disable\n  /info\n";
}

void Receiver::stop() {
    lo_server_thread_stop(server_thread_);
    lo_server_thread_free(server_thread_);
}

int Receiver::oscHandler(const char *path, const char *types, 
                        lo_arg **argv, int argc, lo_message msg, void *user_data) {
    Receiver* receiver = static_cast<Receiver*>(user_data);
    Command cmd{};
    
    lo_address addr = lo_message_get_source(msg);
    cmd.senderIp = lo_address_get_hostname(addr);
    cmd.senderPort = std::stoi(lo_address_get_port(addr));

    try {
        if (strcmp(path, "/rotate") == 0) {
            // Allow numeric types for first two args, require int for direction
            if (argc != 3 || 
                !(types[0] == 'i' || types[0] == 'f') ||  // steps
                !(types[1] == 'i' || types[1] == 'f') ||  // delay
                types[2] != 'i') {                         // direction
                throw std::runtime_error("Invalid arguments. Expected [number] [number] [int]");
            }

            cmd.type = Command::ROTATE;
            
            // Parse steps (int)
            if (types[0] == 'i') cmd.steps = argv[0]->i;
            else cmd.steps = static_cast<int>(argv[0]->f);  // truncate float to int

            // Parse delayUs (float)
            if (types[1] == 'i') cmd.delayUs = static_cast<float>(argv[1]->i);
            else cmd.delayUs = argv[1]->f;

            // Parse direction (bool)
            cmd.direction = argv[2]->i;
            cmd.index = ++receiver->commandIndex_;

            std::cout << "[CMD] ROTATE from " << cmd.senderIp 
                    << " Steps: " << cmd.steps
                    << " μDelay: " << cmd.delayUs 
                    << " Dir: " << (cmd.direction ? "CW" : "CCW") << "\n";
        }
        else if (strcmp(path, "/enable") == 0) {
            cmd.type = Command::ENABLE;
            cmd.index = ++receiver->commandIndex_;
            std::cout << "[CMD] ENABLE from " << cmd.senderIp << "\n";
        }
        else if (strcmp(path, "/disable") == 0) {
            cmd.type = Command::DISABLE;
            cmd.index = ++receiver->commandIndex_;
            std::cout << "[CMD] DISABLE from " << cmd.senderIp << "\n";
        }
        else if (strcmp(path, "/info") == 0) {
            cmd.type = Command::INFO;
            Sender sender;
            sender.sendInfo(cmd.senderIp, 12345, receiver->commandQueue_);
            std::cout << "[CMD] INFO from " << cmd.senderIp << "\n";
            return 0;
        }
        else {
            return 1;
        }

        {
            std::lock_guard<std::mutex> lock(receiver->queueMutex_);
            receiver->commandQueue_.push(cmd);
            receiver->cv_.notify_one();
        }

        Sender sender;
        sender.sendAck(cmd.senderIp, 12345, cmd.index);
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] From " << cmd.senderIp 
                  << ":" << cmd.senderPort << " - " << e.what() << "\n";
    }
    return 0;
}