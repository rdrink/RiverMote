#include "sensors/minimote/ozone.h"

#if MINIMOTE

#include <DFRobot_OzoneSensor.h>

#define COLLECT_NUMBER 20 // Number of samples to collect for smoothing the data, 1-100

static DFRobot_OzoneSensor ozone;
static bool ready = false;

bool ozone_init() {
    // The I2C address depends on the A0 and A1 switches set on the back of the unit
    // A0, A1
    // 0, 0 = OZONE_ADDRESS_0
    // 0, 1 = OZONE_ADDRESS_1
    // 1, 0 = OZONE_ADDRESS_2
    // 1, 1 = OZONE_ADDRESS_3 (seems to be the default)
    ready = ozone.begin(OZONE_ADDRESS_3);
    return ready;
}

double ozone_read() {
    if (!ready) {
        return NAN;
    }
    return ozone.readOzoneData(COLLECT_NUMBER) / 1000.0; // Convert from ppb to ppm
}

#endif // MINIMOTE
