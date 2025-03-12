#include "sender.h"
#include <oscpp/client.hpp>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <array>
#include <iostream>

void Sender::print_connection_info(const std::string& ip, int port) {
    std::cout << "Established connection to: " << ip << ":" << port << "\n";
    std::cout << "Available commands:\n";
    std::cout << "  /rotate <steps> <delay_us> <direction(0|1)>\n";
    std::cout << "  /enable\n  /disable\n  /info\n";
}

void Sender::sendAck(const std::string& ip, int port, int index) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    std::array<char, 1024> buffer;
    OSCPP::Client::Packet packet(buffer.data(), buffer.size());
    packet.openMessage("/ack", 1).int32(index).closeMessage();

    sendto(sock, packet.data(), packet.size(), 0, (sockaddr*)&addr, sizeof(addr));
    close(sock);
    
    print_connection_info(ip, port);
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
}

void Sender::sendInfo(const std::string& ip, int port, const std::queue<Command>& queue) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    std::array<char, 1024> buffer;  // Use stack-allocated buffer
    OSCPP::Client::Packet packet(buffer.data(), buffer.size());    auto msg = packet.openMessage("/info", 1);
    auto arr = msg.openArray();
    
    std::queue<Command> qCopy = queue;
    while (!qCopy.empty()) {
        const Command& cmd = qCopy.front();
        auto cmdArr = arr.openArray();
        cmdArr.int32(cmd.index);
        
        switch (cmd.type) {
            case Command::ROTATE:
                cmdArr.string("rotate")
                      .int32(cmd.steps)
                      .int32(cmd.delayUs)
                      .int32(cmd.direction);
                break;
            case Command::ENABLE:
                cmdArr.string("enable");
                break;
            case Command::DISABLE:
                cmdArr.string("disable");
                break;
        }
        cmdArr.closeArray();
        qCopy.pop();
    }
    arr.closeArray();
    msg.closeMessage();

    sendto(sock, packet.data(), packet.size(), 0, (sockaddr*)&addr, sizeof(addr));
    close(sock);
}