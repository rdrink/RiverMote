#include "sensors/rivermote/spectral.h"

#if RIVERMOTE

#include <AS726X.h>

static AS726X sensor;
static bool initialized = false;
static SpectralData prevData = {};
// byte GAIN = 0;
// byte MEASUREMENT_MODE = 0;

bool spectral_init() {
    return sensor.begin();
    // sensor.begin(Wire, GAIN, MEASUREMENT_MODE); //Initializes the sensor with non default values
}

SpectralData get_spectrum() {
    if (!initialized) {
        return prevData;
    }
    if (!sensor.dataAvailable()) {
        return prevData;
    }

    sensor.takeMeasurements();
    prevData = {
        .violet = sensor.getCalibratedViolet(),
        .blue = sensor.getCalibratedBlue(),
        .green = sensor.getCalibratedGreen(),
        .yellow = sensor.getCalibratedYellow(),
        .orange = sensor.getCalibratedOrange(),
        .red = sensor.getCalibratedRed(),
    };
    return prevData;
}

#endif // RIVERMOTE
