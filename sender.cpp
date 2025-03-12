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
    
    auto msg = packet.openMessage("/info", 1);
    auto main_array = msg.openArray();

    std::queue<Command> q_copy = queue;
    while (!q_copy.empty()) {
        const Command& cmd = q_copy.front();
        
        main_array.int32(cmd.index);
        switch (cmd.type) {
            case Command::ROTATE:
                main_array.string("rotate")
                         .int32(cmd.steps)
                         .int32(cmd.delayUs)
                         .int32(cmd.direction);
                break;
            case Command::ENABLE:
                main_array.string("enable");
                break;
            case Command::DISABLE:
                main_array.string("disable");
                break;
            case Command::INFO:
                // INFO commands shouldn't be in the queue; skip if present
                break;
        }
        
        q_copy.pop();
    }

    main_array.closeArray();
    msg.closeMessage();

    sendto(sock, packet.data(), packet.size(), 0,
          reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    close(sock);
}