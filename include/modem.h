#pragma once

#define LOG_MODEM 0 // Duplicate all modem communications to Serial for debugging

typedef struct ModemGPSData {
    float lat, lng;
    float speed;
    float track;
} ModemGPSData;

/**
 * Initialize the modem with the specified baud rate and maximum retries to establish communication.
 * @param baud baud rate for modem serial communication
 * @param max_retries maximum number of retries to establish communication
 * @return true if the modem was successfully initialized
 */
bool modem_init(unsigned long baud = 115200, uint max_retries = 10);

/**
 * Enable the modem's GPS.
 * @return true if GPS was successfully enabled
 */
bool modem_gps_enable();

/**
 * @return true if GPS is currently enabled
 */
bool modem_gps_is_enabled();

/**
 * Disable the modem's GPS.
 * @return true if GPS was successfully disabled
 */
bool modem_gps_disable();

/**
 * Read the current GPS data from the modem.
 * @return ModemGPSData structure, or all fields zeroed if reading failed
 */
ModemGPSData modem_gps_read();

/**
 * Read the current GPS time from the modem.
 * @return String containing the current time in format `yyyyMMddhhmmss`, or "00000000000000" if GPS is not fixed
 */
String modem_gps_read_time();

/**
 * @return true if the GPS has a valid fix
 */
bool modem_gps_fixed();
