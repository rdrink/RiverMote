#include <ICM_20948.h>
#include <math.h>

#include "pins.h"

#include "sensors/imu.h"

ICM_20948_I2C icm;
static IMUAngles prevAngles = {0.f, 0.f, 0.f};

/**
 * Convert quaternion to Euler angles.
 * @param angles pointer to IMUAngles struct to store the result
 * @param q1 quaternion component 1
 * @param q2 quaternion component 2
 * @param q3 quaternion component 3
 * @param q0 quaternion component 0
 */
static void quat_to_euler(IMUAngles *angles, double q1, double q2, double q3, double q0) {
    // Roll (x)
    double sinr_cosp = 2.0 * (q0 * q1 + q2 * q3);
    double cosr_cosp = 1.0 - 2.0 * (q1 * q1 + q2 * q2);
    angles->roll = degrees(atan2(sinr_cosp, cosr_cosp));
    // Pitch (y)
    double sinp = 2.0 * (q0 * q2 - q3 * q1);
    // Clamp to 90 degrees
    if (fabs(sinp) >= 1) {
        angles->pitch = degrees(copysign(M_PI / 2, sinp));
    } else {
        angles->pitch = degrees(asin(sinp));
    }
    // Yaw (z)
    double siny_cosp = 2.0 * (q0 * q3 + q1 * q2);
    double cosy_cosp = 1.0 - 2.0 * (q2 * q2 + q3 * q3);
    angles->yaw = degrees(atan2(siny_cosp, cosy_cosp));
}

bool imu_init() {
    icm.enableDebugging();
    icm.begin();
    Serial.printf("icm init: %s\n", icm.statusString());
    if (icm.status != ICM_20948_Stat_Ok) {
        return false;
    }
    Serial.println("enabling dmp");
    bool dmpOk = true;
    dmpOk &= (icm.initializeDMP() == ICM_20948_Stat_Ok);
    dmpOk &= (icm.enableDMPSensor(INV_ICM20948_SENSOR_ORIENTATION) == ICM_20948_Stat_Ok);
    dmpOk &= (icm.setDMPODRrate(DMP_ODR_Reg_Quat9, 0) == ICM_20948_Stat_Ok);
    dmpOk &= (icm.enableFIFO() == ICM_20948_Stat_Ok);
    dmpOk &= (icm.enableDMP() == ICM_20948_Stat_Ok);
    dmpOk &= (icm.resetDMP() == ICM_20948_Stat_Ok);
    dmpOk &= (icm.resetFIFO() == ICM_20948_Stat_Ok);
    if (!dmpOk) {
        Serial.println("icm dmp init fail");
    }
    return dmpOk;
}

IMUAngles imu_read() {
    icm_20948_DMP_data_t data;
    icm.readDMPdataFromFIFO(&data);
    // Ensure read was successful
    if (icm.status != ICM_20948_Stat_Ok && icm.status != ICM_20948_Stat_FIFOMoreDataAvail) {
        return prevAngles;
    }
    // Ensure reading contains expected data
    if ((data.header & DMP_header_bitmap_Quat9) <= 0) {
        return prevAngles;
    }

    // Compute q0 and scale to +/- 1
    double q1 = ((double)data.Quat9.Data.Q1) / 1073741824.0; // Divide by 2^30
    double q2 = ((double)data.Quat9.Data.Q2) / 1073741824.0;
    double q3 = ((double)data.Quat9.Data.Q3) / 1073741824.0;
    double q0 = sqrt(1.0 - ((q1 * q1) + (q2 * q2) + (q3 * q3)));
    // Convert quaternion to Euler angles
    IMUAngles angles;
    quat_to_euler(&angles, q1, q2, q3, q0);
    // Save previous angles and return current angles
    prevAngles = angles;
    return angles;

}
