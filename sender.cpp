#include "sender.h"
#include <oscpp/client.hpp>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

void Sender::sendAck(const std::string& ip, int port, int index) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    OSCPP::Client::Packet packet(1024);
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

    OSCPP::Client::Packet packet(1024);
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

    OSCPP::Client::Packet packet(4096);
    auto msg = packet.openMessage("/info", 1);
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