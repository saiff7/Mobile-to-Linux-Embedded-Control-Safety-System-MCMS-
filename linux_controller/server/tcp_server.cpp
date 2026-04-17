#include "tcp_server.h"
#include "logger.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <sstream>

TCPServer::TCPServer(int port)
    : port_(port), serverFd_(-1), clientFd_(-1), running_(false) {}

TCPServer::~TCPServer() {
    stop();
}

void TCPServer::start(DataCallback callback) {
    // Create socket
    serverFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd_ < 0) throw std::runtime_error("Failed to create socket");

    // Allow reuse of port
    int opt = 1;
    setsockopt(serverFd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind to port
    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port_);

    if (bind(serverFd_, (sockaddr*)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("Failed to bind to port " + std::to_string(port_));

    // Listen
    if (listen(serverFd_, 1) < 0)
        throw std::runtime_error("Failed to listen on socket");

    running_ = true;
    Logger::getInstance().info("TCP Server listening on port " + std::to_string(port_));

    // Start accept loop in background thread
    acceptThread_ = std::thread(&TCPServer::acceptLoop, this, callback);
}

void TCPServer::acceptLoop(DataCallback callback) {
    while (running_) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);

        Logger::getInstance().info("Waiting for phone connection...");
        int clientFd = accept(serverFd_, (sockaddr*)&clientAddr, &clientLen);

        if (clientFd < 0) {
            if (running_) Logger::getInstance().error("Accept failed");
            break;
        }

        // Log who connected
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        Logger::getInstance().info("Phone connected from: " + std::string(clientIP));

        clientFd_ = clientFd;
        handleClient(clientFd, callback);
        close(clientFd);
        clientFd_ = -1;
        Logger::getInstance().warn("Phone disconnected. Waiting for reconnect...");
    }
}

void TCPServer::handleClient(int clientFd, DataCallback callback) {
    char buffer[4096];
    std::string accumulator;

    while (running_) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

        if (bytesRead <= 0) {
            // Client disconnected or error
            break;
        }

        accumulator += std::string(buffer, bytesRead);

        // Process complete JSON messages (newline-delimited)
        size_t pos;
        while ((pos = accumulator.find('\n')) != std::string::npos) {
            std::string message = accumulator.substr(0, pos);
            accumulator = accumulator.substr(pos + 1);

            if (!message.empty() && callback) {
                callback(message);
            }
        }
    }
}

void TCPServer::stop() {
    running_ = false;
    if (serverFd_ >= 0) { close(serverFd_); serverFd_ = -1; }
    if (clientFd_ >= 0) { close(clientFd_); clientFd_ = -1; }
    if (acceptThread_.joinable()) acceptThread_.join();
    Logger::getInstance().info("TCP Server stopped.");
}

bool TCPServer::isRunning() const { return running_; }
