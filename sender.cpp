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
        // Case: Empty queue
        auto msg = packet.openMessage("/info", 1); // 1 argument
        msg.string("no commands in the queue");
        msg.closeMessage();
    } else {
        // Case: Non-empty queue
        // First pass: Calculate total arguments
        int totalArgs = 0;
        std::queue<Command> temp = q_copy;
        while (!temp.empty()) {
            const Command& cmd = temp.front();
            if (cmd.type == Command::ROTATE) {
                totalArgs += 5; // index (i), "rotate" (s), steps (i), delay (i), direction (i)
            } else if (cmd.type == Command::ENABLE || cmd.type == Command::DISABLE) {
                totalArgs += 2; // index (i), "enable"/"disable" (s)
            }
            temp.pop();
        }

        // Second pass: Build the OSC message
        auto msg = packet.openMessage("/info", totalArgs);
        while (!q_copy.empty()) {
            const Command& cmd = q_copy.front();
            msg.int32(cmd.index); // Add command index (int32)
            switch (cmd.type) {
                case Command::ROTATE:
                    msg.string("rotate")       // Add type (string)
                       .int32(cmd.steps)       // Steps (int32)
                       .int32(cmd.delayUs)     // Delay (int32)
                       .int32(cmd.direction); // Direction (int32)
                    break;
                case Command::ENABLE:
                    msg.string("enable"); // Type (string)
                    break;
                case Command::DISABLE:
                    msg.string("disable"); // Type (string)
                    break;
                case Command::INFO:
                    break; // Skip INFO commands
            }
            q_copy.pop();
        }
        msg.closeMessage();
    }

    sendto(sock, packet.data(), packet.size(), 0,
          reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    close(sock);
}