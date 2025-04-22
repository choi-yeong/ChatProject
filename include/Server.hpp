#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <string>

class Server {
    int serverSocket;
    int port;
    bool running;
    std::vector<int> clientSockets;

public:
    Server(int port);
    void start();
    void acceptClient();
    void broadcast(const std::string& message);
    void sendMessage(int clientSocket, const std::string& message);
    void stop();
};

#endif
