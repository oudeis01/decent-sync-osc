#include "sender.h"
#include <oscpp/client.hpp>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <array>
#include <iostream>
#include <sstream>

void Sender::print_connection_info(const std::string& ip, int port) {
    std::cout << "\n[NEW CLIENT]\n"
              << "  Address: " << ip << ":" << port << "\n"
              << "  First connection established\n\n";
}

void Sender::sendAck(const std::string& ip, int port, int index) {
    const int RESPONSE_PORT = 12345;
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
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    std::array<char, 4096> buffer;
    OSCPP::Client::Packet packet(buffer.data(), buffer.size());
    
    auto msg = packet.openMessage("/info", 1);
    std::stringstream ss;

    std::queue<Command> q_copy = queue;
    if (!q_copy.empty()) {
        const Command& cmd = q_copy.front();
        ss << "CMD #" << cmd.index << " ";
        if (cmd.type == Command::ROTATE) {
            ss << "ROTATE " << cmd.steps << " " << cmd.delayUs << " " << cmd.direction;
        } else if (cmd.type == Command::ENABLE) {
            ss << "ENABLE";
        } else if (cmd.type == Command::DISABLE) {
            ss << "DISABLE";
        }
        ss << "\n";

        q_copy.pop();
    }
    else{
        ss << "No commands in queue";
    }
    msg.string(ss.str().c_str());
    msg.closeMessage();

    sendto(sock, packet.data(), packet.size(), 0,
          reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    close(sock);
}