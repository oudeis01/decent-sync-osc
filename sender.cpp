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
    constexpr size_t BUFFER_SIZE = 8192;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(RESPONSE_PORT);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    std::array<char, BUFFER_SIZE> buffer;
    OSCPP::Client::Packet packet(buffer.data(), buffer.size());
    
    // Open message with 1 argument (the array)
    auto msg = packet.openMessage("/info", 1);
    auto main_array = msg.openArray();  // Array type is automatic

    std::queue<Command> q_copy = queue;
    while (!q_copy.empty()) {
        const Command& cmd = q_copy.front();
        
        // Create nested array for each command
        auto cmd_entry = main_array.openArray();
        cmd_entry.int32(cmd.index)
                 .string(cmd.type == Command::ROTATE ? "rotate" : 
                         cmd.type == Command::ENABLE ? "enable" : "disable");
        
        if (cmd.type == Command::ROTATE) {
            cmd_entry.int32(cmd.steps)
                     .int32(cmd.delayUs)
                     .int32(cmd.direction);
        }
        cmd_entry.closeArray();
        
        q_copy.pop();
    }

    main_array.closeArray();
    msg.closeMessage();

    sendto(sock, packet.data(), packet.size(), 0,
          reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    close(sock);
}