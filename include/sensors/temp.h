#pragma once

/**
 * Initialize the temperature sensor.
 * @return true on successful initialization
 */
bool temp_init();

/**
 * @return the current temperature reading in °C
 */
float temp_read();
