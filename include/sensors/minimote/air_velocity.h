#pragma once

#if MINIMOTE

/**
 * Initialize the air velocity sensor.
 * @return true if initialization was successful
 */
bool velo_init();

/**
 * @return the current air velocity in meters per second
 */
float velo_read();

#endif // MINIMOTE
