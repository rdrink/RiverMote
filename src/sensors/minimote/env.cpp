#include "sensors/minimote/env.h"

#if MINIMOTE

#include <DFRobot_BME280.h>
#include <DFRobot_ENS160.h>

static DFRobot_ENS160_I2C ens(&Wire, 0x52); // NOT the default address! It conflicts with our UV sensor
static DFRobot_BME280_IIC bme(&Wire, 0x76);
static bool ready = false;

bool env_init() {
    bme.reset();
    ready = bme.begin() == DFRobot_BME280_IIC::eStatusOK;
    ready &= ens.begin() == NO_ERR;
    if (ready) {
        ens.setPWRMode(ENS160_STANDARD_MODE);
        ens.setTempAndHum(bme.getTemperature(), bme.getHumidity());
    }
    return ready;
}

EnvData env_read() {
    if (!ready) {
        return {NAN, NAN, NAN, NAN};
    }

    uint32_t press = bme.getPressure();
    EnvData data = {
        .temp = bme.getTemperature(),
        .hum = bme.getHumidity(),
        .baro = static_cast<float>(press / 100.f), // Concert Pa to hPa
        .alt = bme.calAltitude(1015.f, press),
        .aqi = static_cast<float>(ens.getAQI()),
        .voc = static_cast<float>(ens.getTVOC() / 1000.f), // Convert ppb to ppm
        .co2 = static_cast<float>(ens.getECO2()),
    };
    
    // Check for validity
    bme.lastOperateStatus
    if (data.temp < -40.f || data.temp > 50.f) {
        data.temp = NAN;
    }
    if (data.hum < 0.f || data.hum > 100.f) {
        data.hum = NAN;
    }
    if (ens.getENS160Status() != ens.eNormalOperation) {
        data.aqi = NAN;
        data.voc = NAN;
        data.co2 = NAN;
    }
    return data;
}

#endif // MINIMOTE
