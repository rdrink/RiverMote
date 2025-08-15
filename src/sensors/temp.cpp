#include <OneWire.h>
#include <DallasTemperature.h>

#include "pins.h"

#include "sensors/temp.h"

OneWire oneWire(PIN_TEMP);
DallasTemperature temp(&oneWire);

int count = 0;

bool temp_init() {
    temp.begin();
    uint8_t devices = temp.getDeviceCount();
    Serial.printf("found %d temperature devices\n", devices);
    return devices > 0;
}

float temp_read() {
    temp.requestTemperatures();
    return temp.getTempCByIndex(0);
}
