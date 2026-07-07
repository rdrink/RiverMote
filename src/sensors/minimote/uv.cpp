#include "sensors/minimote/uv.h"

#if MINIMOTE

// https://optoelectronics.liteon.com/upload/download/DS86-2015-0004/LTR-390UV_Final_%20DS_V1%201.pdf
#include <Adafruit_LTR390.h>

static Adafruit_LTR390 ltr = Adafruit_LTR390();
static bool ready = false;

bool uv_init() {
    ready = ltr.begin();
    ltr.setMode(LTR390_MODE_UVS);
    ltr.setGain(LTR390_GAIN_3);
    ltr.setResolution(LTR390_RESOLUTION_20BIT);
    return ready;
}

float uv_read() {
    if (!ready) {
        return NAN;
    }
    // Conversion factor from ADC counts to UVI in 20bit mode; see datasheet page 6
    return static_cast<float>(ltr.readUVS() / 2300.f);
}

#endif // MINIMOTE
