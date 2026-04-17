#pragma once
#include <string>
#include <cstdint>

struct SensorData {
    double acc_x      = 0.0;   // Accelerometer X (m/s²)
    double acc_y      = 0.0;   // Accelerometer Y (m/s²)
    double acc_z      = 0.0;   // Accelerometer Z (m/s²)
    double gyro_x     = 0.0;   // Gyroscope X (rad/s)
    double gyro_y     = 0.0;   // Gyroscope Y (rad/s)
    double gyro_z     = 0.0;   // Gyroscope Z (rad/s)
    double sound_level = 0.0;  // Microphone amplitude (0.0 - 1.0)
    int64_t timestamp  = 0;    // Unix ms timestamp from phone

    // Computed helpers
    double totalAcceleration() const {
        return std::sqrt(acc_x*acc_x + acc_y*acc_y + acc_z*acc_z);
    }

    double totalGyro() const {
        return std::sqrt(gyro_x*gyro_x + gyro_y*gyro_y + gyro_z*gyro_z);
    }
};
