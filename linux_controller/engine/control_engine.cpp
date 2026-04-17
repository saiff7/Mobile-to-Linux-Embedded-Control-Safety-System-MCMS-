#include "control_engine.h"
#include "logger.h"
#include <cmath>
#include <sstream>
#include <iomanip>

ControlEngine::ControlEngine()
    : currentState_(static_cast<int>(SystemState::IDLE)) {
    Logger::getInstance().info("Control Engine initialized. State: IDLE");
}

bool ControlEngine::process(const SensorData& data) {
    SystemState prev = static_cast<SystemState>(currentState_.load());

    // Don't process in EMERGENCY_STOP — must be manually reset
    if (prev == SystemState::EMERGENCY_STOP) return false;

    SystemState next = evaluate(data);

    if (next != prev) {
        std::ostringstream reason;
        reason << std::fixed << std::setprecision(2);
        reason << "acc=" << data.totalAcceleration()
               << " gyro=" << data.totalGyro()
               << " sound=" << data.sound_level;

        logTransition(prev, next, reason.str());
        currentState_.store(static_cast<int>(next));
        return true;
    }
    return false;
}

SystemState ControlEngine::evaluate(const SensorData& data) {
    double acc   = data.totalAcceleration();
    double gyro  = data.totalGyro();
    double sound = data.sound_level;

    // ── Priority order: highest severity wins ────────────────────
    if (acc >= thresholds_.accelEmergency || sound >= thresholds_.soundEmergency) {
        return SystemState::EMERGENCY_STOP;
    }

    if (acc >= thresholds_.accelAlert || gyro >= thresholds_.gyroAlert
        || sound >= thresholds_.soundAlert) {
        return SystemState::ALERT;
    }

    if (acc >= thresholds_.accelWarning || gyro >= thresholds_.gyroWarning) {
        return SystemState::WARNING;
    }

    return SystemState::MONITORING;
}

void ControlEngine::logTransition(SystemState from, SystemState to,
                                   const std::string& reason) {
    std::string msg = "STATE: " + getStateName(from) + " → " +
                      getStateName(to) + "  [" + reason + "]";

    if (to == SystemState::EMERGENCY_STOP) {
        Logger::getInstance().alert("🚨 EMERGENCY STOP! " + msg);
        Logger::getInstance().alert("All virtual motors HALTED.");
    } else if (to == SystemState::ALERT) {
        Logger::getInstance().alert("⚠️  ALERT triggered. " + msg);
    } else if (to == SystemState::WARNING) {
        Logger::getInstance().warn("WARNING threshold reached. " + msg);
    } else if (to == SystemState::MONITORING) {
        Logger::getInstance().info("System nominal. " + msg);
    }
}

SystemState ControlEngine::getState() const {
    return static_cast<SystemState>(currentState_.load());
}

std::string ControlEngine::getStateName(SystemState s) const {
    switch(s) {
        case SystemState::IDLE:            return "IDLE";
        case SystemState::MONITORING:      return "MONITORING";
        case SystemState::WARNING:         return "WARNING";
        case SystemState::ALERT:           return "ALERT";
        case SystemState::EMERGENCY_STOP:  return "EMERGENCY_STOP";
        default:                           return "UNKNOWN";
    }
}

std::string ControlEngine::getStateName() const {
    return getStateName(getState());
}

void ControlEngine::reset() {
    SystemState prev = getState();
    currentState_.store(static_cast<int>(SystemState::MONITORING));
    Logger::getInstance().info("Manual RESET — " + getStateName(prev) + " → MONITORING");
}

void ControlEngine::setIdle() {
    currentState_.store(static_cast<int>(SystemState::IDLE));
    Logger::getInstance().info("Phone disconnected — State → IDLE");
}
