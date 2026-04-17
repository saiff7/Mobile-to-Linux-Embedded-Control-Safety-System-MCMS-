#pragma once
#include <string>
#include <functional>
#include <atomic>
#include <thread>

// Callback type: called whenever a full JSON message arrives
using DataCallback = std::function<void(const std::string& jsonData)>;

class TCPServer {
public:
    TCPServer(int port);
    ~TCPServer();

    void start(DataCallback callback);
    void stop();
    bool isRunning() const;

private:
    int port_;
    int serverFd_;
    int clientFd_;
    std::atomic<bool> running_;
    std::thread acceptThread_;

    void acceptLoop(DataCallback callback);
    void handleClient(int clientFd, DataCallback callback);
};
