#pragma once

#if MINIMOTE

/**
 * Initializes the ozone sensor.
 * @return true if succesful
 */
bool ozone_init();

/**
 * @return the ozone concentration in ppm (0-10)
 */
double ozone_read();

#endif // MINIMOTE
