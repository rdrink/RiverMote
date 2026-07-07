#include "sensors/rivermote/compass.h"

#if RIVERMOTE

#include <math.h>
#include <SparkFun_MMC5983MA_Arduino_Library.h>

static SFE_MMC5983MA mag;
static bool magInitialized = false;

bool compass_init() {
    if (!mag.begin()) {
        return false;
    }
    if (!mag.softReset()) {
        return false;
    }
    magInitialized = true;
    return true;
}

float compass_read() {
    if (!magInitialized) {
        return 0.f;
    }

    uint32_t rawX = 0, rawY = 0, rawZ = 0;
    double scaledX = 0.0, scaledY = 0.0, scaledZ = 0.0;
    double heading = 0.0;

    // Read all three channels simultaneously
    mag.getMeasurementXYZ(&rawX, &rawY, &rawZ);

    // The magnetic field values are 18-bit unsigned. The _approximate_ zero (mid) point is 2^17 (131072).
    // Here we scale each field to +/- 1.0 to make it easier to calculate an approximate heading.
    //
    // Please note: to properly correct and calibrate the X, Y and Z channels, you need to determine true
    // offsets (zero points) and scale factors (gains) for all three channels. Futher details can be found at:
    // https://thecavepearlproject.org/2015/05/22/calibrating-any-compass-or-accelerometer-for-arduino/
    scaledX = (double)rawX - 131072.0;
    scaledX /= 131072.0;
    scaledY = (double)rawX - 131072.0; // !!! IS THIS SUPPOSED TO BE 'rawY' ???
    scaledY /= 131072.0;
    scaledZ = (double)rawZ - 131072.0;
    scaledZ /= 131072.0;

    // Magnetic north is oriented with the Y axis
    // Convert the X and Y fields into heading using atan2
    heading = atan2(scaledX, 0 - scaledY);
    // Convert to degrees and return
    return degrees(heading) + 180.f;
}

#endif // RIVERMOTE
