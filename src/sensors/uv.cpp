#include "sensors/uv.h"

#if MINIMOTE

#include <Adafruit_LTR390.h>

Adafruit_LTR390 ltr = Adafruit_LTR390();

void uv_init(){

    if ( ! ltr.begin() ) {
        Serial.println("Couldn't find LTR sensor!");
        while (1) delay(10);
    }
    Serial.println("Found LTR sensor!");

    ltr.setMode(LTR390_MODE_UVS);
    if (ltr.getMode() == LTR390_MODE_ALS) {
        Serial.println("In ALS mode");
    } else {
        Serial.println("In UVS mode");
    }

    ltr.setGain(LTR390_GAIN_3);
    Serial.print("Gain : ");
    switch (ltr.getGain()) {
        case LTR390_GAIN_1: Serial.println(1); break;
        case LTR390_GAIN_3: Serial.println(3); break;
        case LTR390_GAIN_6: Serial.println(6); break;
        case LTR390_GAIN_9: Serial.println(9); break;
        case LTR390_GAIN_18: Serial.println(18); break;
    }

    ltr.setResolution(LTR390_RESOLUTION_16BIT);
    Serial.print("Resolution : ");
    switch (ltr.getResolution()) {
        case LTR390_RESOLUTION_13BIT: Serial.println(13); break;
        case LTR390_RESOLUTION_16BIT: Serial.println(16); break;
        case LTR390_RESOLUTION_17BIT: Serial.println(17); break;
        case LTR390_RESOLUTION_18BIT: Serial.println(18); break;
        case LTR390_RESOLUTION_19BIT: Serial.println(19); break;
        case LTR390_RESOLUTION_20BIT: Serial.println(20); break;
    }

    ltr.setThresholds(100, 1000);
    ltr.configInterrupt(true, LTR390_MODE_UVS);
}

float uv_read() {
    if (ltr.newDataAvailable()) {
        return ltr.readUVS();
    }
}

#endif