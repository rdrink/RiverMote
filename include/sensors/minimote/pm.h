#pragma once

#if MINIMOTE

typedef struct PMData {
    float pm10, pm2_5, pm1_0; // 10µm, 2.5µm, 1.0µm particulate matter concentrations
} PMData;

/**
 * Initialize the particulate matter sensor.
 * @return true on successful initialization
 */
bool pm_init();

/**
 * Read data from the particulate matter sensor.
 * @return PMData structure containing the readings
 */
PMData pm_read();

/**
 * Refresh readings for the particulate matter sensor.
 * @note This function must be called at least once a second to keep processing data.
 */
void pm_loop();

#endif // MINIMOTE
