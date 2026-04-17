#pragma once
#include <atomic>
#include <thread>
#include <mutex>
#include <functional>
#include "engine/control_engine.h"

class Simulator {
public:
    Simulator(ControlEngine& engine);
    ~Simulator();

    void start();
    void stop();

private:
    ControlEngine& engine_;
    std::atomic<bool> running_;
    std::thread simThread_;

    void simulationLoop();
    void printMotorStatus(SystemState state);
};
