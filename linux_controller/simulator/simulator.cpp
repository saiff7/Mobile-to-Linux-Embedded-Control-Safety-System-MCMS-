#include "simulator.h"
#include "logger.h"
#include <sstream>
#include <chrono>
#include <thread>

Simulator::Simulator(ControlEngine& engine)
    : engine_(engine), running_(false) {}

Simulator::~Simulator() { stop(); }

void Simulator::start() {
    running_ = true;
    simThread_ = std::thread(&Simulator::simulationLoop, this);
    Logger::getInstance().info("Simulator started — virtual motor output active.");
}

void Simulator::stop() {
    running_ = false;
    if (simThread_.joinable()) simThread_.join();
    Logger::getInstance().info("Simulator stopped.");
}

void Simulator::simulationLoop() {
    SystemState lastState = SystemState::IDLE;

    while (running_) {
        SystemState current = engine_.getState();

        // Only print when state changes
        if (current != lastState) {
            printMotorStatus(current);
            lastState = current;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void Simulator::printMotorStatus(SystemState state) {
    std::string border = "╔══════════════════════════════════════╗";
    std::string footer = "╚══════════════════════════════════════╝";

    switch (state) {
        case SystemState::IDLE:
            Logger::getInstance().info(border);
            Logger::getInstance().info("║  🔵 VIRTUAL MACHINE: STANDBY          ║");
            Logger::getInstance().info("║  Motor A: OFF  Motor B: OFF           ║");
            Logger::getInstance().info("║  Motor C: OFF  Motor D: OFF           ║");
            Logger::getInstance().info("║  Pump: OFF  Valve: CLOSED             ║");
            Logger::getInstance().info(footer);
            break;

        case SystemState::MONITORING:
            Logger::getInstance().info(border);
            Logger::getInstance().info("║  🟢 VIRTUAL MACHINE: RUNNING          ║");
            Logger::getInstance().info("║  Motor A: ON   Motor B: ON            ║");
            Logger::getInstance().info("║  Motor C: ON   Motor D: ON            ║");
            Logger::getInstance().info("║  Pump: ON   Valve: OPEN               ║");
            Logger::getInstance().info(footer);
            break;

        case SystemState::WARNING:
            Logger::getInstance().warn(border);
            Logger::getInstance().warn("║  🟡 VIRTUAL MACHINE: CAUTION          ║");
            Logger::getInstance().warn("║  Motor A: 50%  Motor B: 50%           ║");
            Logger::getInstance().warn("║  Motor C: 50%  Motor D: 50%           ║");
            Logger::getInstance().warn("║  Pump: LOW  Valve: THROTTLED          ║");
            Logger::getInstance().warn(footer);
            break;

        case SystemState::ALERT:
            Logger::getInstance().alert(border);
            Logger::getInstance().alert("║  🟠 VIRTUAL MACHINE: ALERT MODE       ║");
            Logger::getInstance().alert("║  Motor A: OFF  Motor B: OFF           ║");
            Logger::getInstance().alert("║  Motor C: 20%  Motor D: 20%           ║");
            Logger::getInstance().alert("║  Pump: OFF  Valve: CLOSING            ║");
            Logger::getInstance().alert(footer);
            break;

        case SystemState::EMERGENCY_STOP:
            Logger::getInstance().alert(border);
            Logger::getInstance().alert("║  🔴 EMERGENCY STOP — ALL HALTED       ║");
            Logger::getInstance().alert("║  Motor A: KILL Motor B: KILL          ║");
            Logger::getInstance().alert("║  Motor C: KILL Motor D: KILL          ║");
            Logger::getInstance().alert("║  Pump: KILL  Valve: SEALED            ║");
            Logger::getInstance().alert("║  ⚡ SAFETY RELAY TRIGGERED            ║");
            Logger::getInstance().alert(footer);
            break;
    }
}
