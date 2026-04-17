#pragma once
#include "sensor_data.h"
#include <string>
#include <atomic>
#include <mutex>

// ── State Machine States ──────────────────────────────────────────
enum class SystemState {
    IDLE,             // No phone connected / system standby
    MONITORING,       // Phone connected, data flowing, all normal
    WARNING,          // Threshold approaching — elevated attention
    ALERT,            // Threshold exceeded — issue warning
    EMERGENCY_STOP    // Critical condition — halt everything
};

// ── Thresholds (tunable) ──────────────────────────────────────────
struct Thresholds {
    double accelWarning       = 12.0;   // m/s²  — gentle shake
    double accelAlert         = 18.0;   // m/s²  — hard shake
    double accelEmergency     = 25.0;   // m/s²  — violent shake → E-STOP
    double gyroWarning        = 1.5;    // rad/s — slow tilt
    double gyroAlert          = 3.0;    // rad/s — fast rotation
    double soundAlert         = 0.75;   // 0–1   — loud noise spike
    double soundEmergency     = 0.92;   // 0–1   — extremely loud
};

class ControlEngine {
public:
    ControlEngine();

    // Feed new sensor data — returns true if state changed
    bool process(const SensorData& data);

    SystemState getState() const;
    std::string getStateName() const;
    std::string getStateName(SystemState s) const;

    void reset();   // Force back to MONITORING
    void setIdle(); // Called on disconnect

private:
    std::atomic<int> currentState_;   // stores SystemState as int
    Thresholds thresholds_;
    mutable std::mutex mtx_;

    SystemState evaluate(const SensorData& data);
    void logTransition(SystemState from, SystemState to, const std::string& reason);
};
