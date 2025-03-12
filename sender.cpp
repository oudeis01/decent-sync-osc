#include "sender.h"
#include <oscpp/client.hpp>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <array>
#include <iostream>
#include <sstream>

constexpr int RESPONSE_PORT = 12345;
constexpr size_t MAX_BUFFER_SIZE = 16384;

void Sender::print_connection_info(const std::string& ip, int port) {
    std::cout << "\n[NEW CLIENT]\n"
              << "  Address: " << ip << ":" << port << "\n"
              << "  First connection established\n\n";
}

void Sender::sendAck(const std::string& ip, int port, int index) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(RESPONSE_PORT);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    std::array<char, 1024> buffer;
    OSCPP::Client::Packet packet(buffer.data(), buffer.size());
    packet.openMessage("/ack", 1).int32(index).closeMessage();

    sendto(sock, packet.data(), packet.size(), 0, (sockaddr*)&addr, sizeof(addr));
    close(sock);
}

void Sender::sendDone(const std::string& ip, int port, int index) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    std::array<char, 1024> buffer;  // Use stack-allocated buffer
    OSCPP::Client::Packet packet(buffer.data(), buffer.size());
    packet.openMessage("/done", 1).int32(index).closeMessage();

    sendto(sock, packet.data(), packet.size(), 0, (sockaddr*)&addr, sizeof(addr));
    close(sock);

    std::cout << "Completed command #" << index << " for "
              << ip << ":" << port << "\n";
}


void Sender::sendInfo(const std::string& ip, int port, const std::queue<Command>& queue) {
    constexpr int RESPONSE_PORT = 12345;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(RESPONSE_PORT);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    std::array<char, MAX_BUFFER_SIZE> buffer;
    OSCPP::Client::Packet packet(buffer.data(), buffer.size());

    std::queue<Command> q_copy = queue;

    if (q_copy.empty()) {
        // Empty queue case
        auto msg = packet.openMessage("/info", 1);
        msg.string("no commands in the queue");
        msg.closeMessage();
    } else {
        // Non-empty queue: use bundle with timestamp
        auto bundle = packet.openBundle(0ULL); // Timestamp 0 = immediate execution
        while (!q_copy.empty()) {
            const Command& cmd = q_copy.front();
            
            // Create message for each command
            auto cmdMsg = bundle.openMessage("/info", 0);
            cmdMsg.int32(cmd.index);
            
            switch (cmd.type) {
                case Command::ROTATE:
                    cmdMsg.string("rotate")
                          .int32(cmd.steps)
                          .int32(cmd.delayUs)
                          .int32(cmd.direction);
                    break;
                case Command::ENABLE:
                    cmdMsg.string("enable");
                    break;
                case Command::DISABLE:
                    cmdMsg.string("disable");
                    break;
                case Command::INFO:
                    break; // Skip
            }
            cmdMsg.closeMessage();
            q_copy.pop();
        }
    }

    sendto(sock, packet.data(), packet.size(), 0,
          reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    close(sock);
}