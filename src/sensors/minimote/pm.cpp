#include "sensors/minimote/pm.h"

#if MINIMOTE

#if defined(NO_BMV080)

bool pm_init() {
    return false;
}

PMData pm_read() {
    return {};
}

void pm_loop() {
}

#else

#include <SparkFun_BMV080_Arduino_Library.h>

static SparkFunBMV080 bmv080;
static bool ready = false;

bool pm_init() {
    if (!bmv080.begin()) {
        return false;
    }
    if (!bmv080.init()) {
        return false;
    }
    // Take a measurement every 60 seconds in duty cycle mode
    ready = bmv080.setDutyCyclingPeriod(60);
    ready &= bmv080.setMode(SF_BMV080_MODE_DUTY_CYCLE);
    return ready;
}

PMData pm_read() {
    if (!ready || bmv080.isObstructed()) {
        return {NAN, NAN, NAN};
    }
    // We ignore the return value of this because it could return false if there is stale data, which is fine-ish
    bmv080.readSensor();
    return PMData{
        .pm10 = bmv080.PM10(),
        .pm2_5 = bmv080.PM25(),
        .pm1_0 = bmv080.PM1()
    };
}

void pm_loop() {
    // This calls `bmv080_serve_interrupt` internally, which needs to be called once a second in duty cycle mode
    bmv080.readSensor();
}

#endif // NO_BMV080

#endif // MINIMOTE
