#pragma once

typedef struct IMUAngles {
    float roll, pitch, yaw;
} IMUAngles;

/**
 * Initialize the IMU.
 * @return true on successful initialization
 */
bool imu_init();

/**
 * Read the current angles from the IMU.
 * @return IMUAngles struct containing roll, pitch, and yaw in degrees
 */
IMUAngles imu_read();
