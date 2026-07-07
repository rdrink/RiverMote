#pragma once

#if MINIMOTE

/**
 * Initialize the chamber temperature sensor.
 * @return true on successful initialization
 */
bool chamber_init();

/**
 * Read the current temperature from the chamber sensor.
 * @return temperature in celsius, or -1 on error
 */
float chamber_read();

#endif // MINIMOTE
