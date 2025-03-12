#ifndef RECEIVER_H
#define RECEIVER_H

#include <queue>
#include <mutex>
#include <atomic>
#include <string>
#include <netinet/in.h>
#include <condition_variable>
#include <thread> 
#include <iostream>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <unordered_set>

// Forward declaration
namespace OSCPP {
namespace Server {
    class Packet;
}
}

struct Command {
    int index;
    std::string senderIp;
    int senderPort;
    enum Type { ROTATE, ENABLE, DISABLE, INFO } type;
    int steps;
    int delayUs;
    bool direction;
};


class Receiver {
public:
    Receiver(int port, std::queue<Command>& queue, std::mutex& mutex,
             std::atomic<int>& cmdIndex, std::condition_variable& cv);
    ~Receiver();
    void processPacket(const OSCPP::Server::Packet& packet, sockaddr_in& cliaddr);
    std::string getLocalIp() const;
    void start();
    void stop();

private:
    void run();
    void process_packet(const OSCPP::Server::Packet& packet, sockaddr_in& cliaddr);
    void track_connection(const std::string& ip, int port);

    int port_;
    int sockfd_;
    std::queue<Command>& commandQueue_;
    std::mutex& queueMutex_;
    std::atomic<int>& commandIndex_;
    std::condition_variable& cv_;
    bool running_;
    std::thread thread_;
    std::unordered_set<std::string> connected_clients_;
};

#endif