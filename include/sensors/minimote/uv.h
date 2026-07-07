#pragma once

#if MINIMOTE

/**
 * Initialize the UV sensor.
 * @return true on successful initialization
 */
bool uv_init();

/**
 * @return UV irradiance in UVI
 */
float uv_read();

#endif // MINIMOTE
