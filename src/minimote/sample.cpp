#include "minimote/sample.h"

#if MINIMOTE

#include <Arduino.h>
#include <ArduinoJson.h>
#include <sys/time.h>

#include "sensors/minimote/air_velocity.h"
#include "sensors/minimote/chamber.h"
#include "sensors/minimote/env.h"
#include "sensors/minimote/ozone.h"
#include "sensors/minimote/pm.h"
#include "sensors/minimote/uv.h"
#include "sensors/tds.h"
#include "sensors/temp.h"
#include "sensors/turbidity.h"
#include "pmu.h"

// Check a sensor's reading before accumulating it.
// If any single reading is invalid, the average is invalid, so set to NAN and skip accumulation (which would skew the average)
#define CHECK_AND_ACCUM(SENSOR, READ_EXPR) do { \
    float val = (READ_EXPR); \
    if (isnan(val) || isnan(acc.SENSOR)) { \
        acc.SENSOR = NAN; \
    } else { \
        acc.SENSOR += val; \
    } \
} while(0)

#define ADD_AVERAGE(SENSOR) do { \
    if (!isnan(acc.SENSOR) && acc.count > 0) { \
        doc[#SENSOR] = (acc.SENSOR / acc.count); \
    } else { \
        doc[#SENSOR] = nullptr; \
    } \
} while(0)

typedef struct SampleAccumulator {
    float water_temp, turbidity, tds;
    float air_velocity, air_velocity_peak;
    float air_temp, humidity, baro, alt;
    float aqi, voc, co2;
    float uv;
    double ozone;
    float pm1_0, pm2_5, pm10;
    float chamber_temp;
    uint32_t count;
} SampleAccumulator;

static SampleAccumulator acc = {}; // Everything initializes to 0 (including count)

void minimote_sample_accumulate() {
    CHECK_AND_ACCUM(water_temp, temp_read());
    CHECK_AND_ACCUM(turbidity, get_turbidity());
    CHECK_AND_ACCUM(tds, get_tds());

    float velo = velo_read();
    CHECK_AND_ACCUM(air_velocity, velo);
    if (isnan(velo) || isnan(acc.air_velocity)) {
        acc.air_velocity_peak = NAN;
    } else if (velo > acc.air_velocity_peak) {
        acc.air_velocity_peak = velo;
    }

    CHECK_AND_ACCUM(ozone, ozone_read());
    EnvData env = env_read();
    CHECK_AND_ACCUM(air_temp, env.temp);
    CHECK_AND_ACCUM(humidity, env.hum);
    CHECK_AND_ACCUM(baro, env.baro);
    CHECK_AND_ACCUM(alt, env.alt);
    CHECK_AND_ACCUM(aqi, env.aqi);
    CHECK_AND_ACCUM(voc, env.voc);
    CHECK_AND_ACCUM(co2, env.co2);
    CHECK_AND_ACCUM(uv, uv_read());

    PMData pm = pm_read();
    CHECK_AND_ACCUM(pm1_0, pm.pm1_0);
    CHECK_AND_ACCUM(pm2_5, pm.pm2_5);
    CHECK_AND_ACCUM(pm10, pm.pm10);
    CHECK_AND_ACCUM(chamber_temp, chamber_read());

    acc.count++;
}

void minimote_sample_get(char *payload, size_t payload_length) {
    // Construct JSON payload
    JsonDocument doc;
    doc["unix_time"] = time(nullptr);
    doc["millis"] = millis();
    doc["battery_v"] = pmu_get_battery_voltage();
    doc["battery_pct"] = pmu_get_battery_percent();

    ADD_AVERAGE(water_temp);
    ADD_AVERAGE(turbidity);
    ADD_AVERAGE(tds);

    ADD_AVERAGE(air_velocity);
    if (!isnan(acc.air_velocity_peak)) {
        doc["air_velocity_peak"] = acc.air_velocity_peak;
    } else {
        doc["air_velocity_peak"] = nullptr;
    }
    ADD_AVERAGE(ozone);
    ADD_AVERAGE(air_temp);
    ADD_AVERAGE(humidity);
    ADD_AVERAGE(baro);
    ADD_AVERAGE(alt);
    ADD_AVERAGE(aqi);
    ADD_AVERAGE(voc);
    ADD_AVERAGE(co2);
    ADD_AVERAGE(uv);

    ADD_AVERAGE(pm1_0);
    ADD_AVERAGE(pm2_5);
    ADD_AVERAGE(pm10);
    ADD_AVERAGE(chamber_temp);

    serializeJson(doc, payload, payload_length);
    // Reset accumulator for next round of sampling
    acc = {};
}

#endif // MINIMOTE
