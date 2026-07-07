#pragma once

#if MINIMOTE

typedef struct EnvData {
    // °C, %, hPa, m
    float temp, hum, baro, alt;
    // AQI-UBA, ppm, ppm
    float aqi, voc, co2;
} EnvData;

/**
 * Initialize the DFR environemental sensor.
 * @return true on successful initialization
 */
bool env_init();

/**
 * @return env sensors values
 */
EnvData env_read();

#endif // MINIMOTE
