#include <iostream>
#include <string>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>

#include "logger/logger.h"
#include "server/tcp_server.h"
#include "engine/sensor_data.h"
#include "engine/control_engine.h"
#include "simulator/simulator.h"

using json = nlohmann::json;

std::atomic<bool> globalRunning(true);
ControlEngine gEngine;

void signalHandler(int sig) {
    std::cout << "\n";
    Logger::getInstance().warn("Shutdown signal (" + std::to_string(sig) + ") — shutting down...");
    globalRunning = false;
}

void onDataReceived(const std::string& jsonStr) {
    try {
        json j = json::parse(jsonStr);

        SensorData data;
        data.acc_x        = j.value("acc_x",       0.0);
        data.acc_y        = j.value("acc_y",        0.0);
        data.acc_z        = j.value("acc_z",        0.0);
        data.gyro_x       = j.value("gyro_x",       0.0);
        data.gyro_y       = j.value("gyro_y",       0.0);
        data.gyro_z       = j.value("gyro_z",       0.0);
        data.sound_level  = j.value("sound_level",  0.0);
        data.timestamp    = j.value("timestamp",    (int64_t)0);

        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "acc=(" << data.acc_x << "," << data.acc_y << "," << data.acc_z << ")"
           << " gyro=(" << data.gyro_x << "," << data.gyro_y << "," << data.gyro_z << ")"
           << " sound=" << data.sound_level
           << " totalAcc=" << data.totalAcceleration();
        Logger::getInstance().info(ss.str());

        // Thread 2: control engine processes data
        gEngine.process(data);

    } catch (const json::exception& e) {
        Logger::getInstance().error("JSON parse error: " + std::string(e.what()));
    }
}

int main() {
    signal(SIGINT,  signalHandler);
    signal(SIGTERM, signalHandler);

    Logger::getInstance().info("╔══════════════════════════════════════╗");
    Logger::getInstance().info("║     MCMS v3.0 — 4-Thread System      ║");
    Logger::getInstance().info("║  Mobile Embedded Control & Safety     ║");
    Logger::getInstance().info("╚══════════════════════════════════════╝");
    Logger::getInstance().info("Thread 1: Network Receiver  — STARTING");
    Logger::getInstance().info("Thread 2: Control Engine    — STARTING");
    Logger::getInstance().info("Thread 3: Logger            — ACTIVE");
    Logger::getInstance().info("Thread 4: Simulator Output  — STARTING");

    // Thread 1: TCP Server (runs its own thread internally)
    TCPServer server(9000);

    // Thread 4: Simulator (watches engine state, prints motor output)
    Simulator simulator(gEngine);

    try {
        simulator.start();          // Thread 4 live
        server.start(onDataReceived); // Thread 1 live

        Logger::getInstance().info("All systems online. Port 9000 ready.");
        Logger::getInstance().info("Awaiting phone connection...");

        // Main thread: heartbeat loop
        int heartbeat = 0;
        while (globalRunning) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            heartbeat++;

            // Print heartbeat every 10s
            if (heartbeat % 10 == 0) {
                Logger::getInstance().info(
                    "♥ Heartbeat #" + std::to_string(heartbeat) +
                    " | State: " + gEngine.getStateName()
                );
            }

            // Auto-reset EMERGENCY_STOP after 10s for demo purposes
            if (gEngine.getState() == SystemState::EMERGENCY_STOP && heartbeat % 10 == 0) {
                Logger::getInstance().warn("Auto-reset from EMERGENCY_STOP after timeout.");
                gEngine.reset();
            }
        }

        simulator.stop();
        server.stop();

    } catch (const std::exception& e) {
        Logger::getInstance().error("Fatal: " + std::string(e.what()));
        return 1;
    }

    Logger::getInstance().info("=== MCMS Shutdown Complete ===");
    return 0;
}
