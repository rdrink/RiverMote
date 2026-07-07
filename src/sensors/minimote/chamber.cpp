#include "sensors/minimote/chamber.h"

#if MINIMOTE

#include <SparkFun_TMP117.h>

static TMP117 temp;
static bool ready = false;

bool chamber_init() {
    ready = temp.begin();
    return ready;
}

float chamber_read() {
    if (!ready || !temp.dataReady()) {
        return NAN;
    }
    float rawTemp = temp.readTempC();
    if (rawTemp < -40.f || rawTemp > 50.f) {
        // Invalid temperature reading, likely due to a sensor error; return NAN to indicate this
        return NAN;
    }
    return rawTemp;
}

#endif // MINIMOTE
