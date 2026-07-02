#pragma once

#if RIVERMOTE

typedef struct SpectralData {
    float violet, blue, green, yellow, orange, red;
} SpectralData;

/**
 * Initialize the spectral sensor.
 */
bool spectral_init();

/**
 * @return the current spectral data
 */
SpectralData get_spectrum();

#endif // RIVERMOTE
