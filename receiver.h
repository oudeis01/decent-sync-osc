#ifndef RECEIVER_H
#define RECEIVER_H

#include <queue>
#include <mutex>
#include <atomic>
#include <string>
#include <netinet/in.h>
#include <condition_variable>

struct Command {
    int index;
    std::string senderIp;
    int senderPort;
    enum Type { ROTATE, ENABLE, DISABLE } type;
    int steps;
    int delayUs;
    bool direction;
};

class Receiver {
public:
    Receiver(int port, std::queue<Command>& queue, std::mutex& mutex,
             std::atomic<int>& cmdIndex, std::condition_variable& cv);
    ~Receiver();
    void start();
    void stop();

private:
    void run();
    void processPacket(const OSCPP::Server::Packet& packet, sockaddr_in& cliaddr);
    
    int port_;
    int sockfd_;
    std::queue<Command>& commandQueue_;
    std::mutex& queueMutex_;
    std::atomic<int>& commandIndex_;
    std::condition_variable& cv_;
    bool running_;
    std::thread thread_;
};

#endif