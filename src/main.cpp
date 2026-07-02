#include <Arduino.h>
#if MINIMOTE
#include <ArduinoJson.h>
#endif
#include <esp_mac.h>
#include <Wire.h>

#include "minimote/minimote.h"
#include "rivermote/rivermote.h"
#include "sensors/air_velocity.h"
#include "sensors/chamber.h"
#include "sensors/compass.h"
#include "sensors/env.h"
//#include "sensors/imu.h"
//#include "sensors/light.h"
#include "sensors/ozone.h"
#include "sensors/pm_sensor.h"
#include "rivermote/spectral.h"
#include "sensors/tds.h"
#include "sensors/temp.h"
#include "sensors/turbidity.h"
#include "sensors/uv.h"
#include "modem.h"
#include "pins.h"
#include "pmu.h"

#define WAIT_FOR_SERIAL 0

#if MINIMOTE
static JsonDocument initted; // Store init status of sensors to report upon connection
#endif

/**
 * Helper function to initialize a sensor and keep status.
 * @param init_func the sensor's initialization function, which should return true if initialization was successful
 * @param sensor_name the human-readable name of the sensor
 */
static void init_sensor(bool (*init_func)(), const char *sensor_name) {
    Serial.printf("Initializing %s:\n", sensor_name);
    bool success = init_func();
    if (success) {
        Serial.printf("- %s initialized\n", sensor_name);
    } else {
        Serial.printf("! %s init failed!\n", sensor_name);
    }
#if MINIMOTE
    initted[sensor_name] = success;
#endif
}

// Overload of `init_sensor` for void init functions
static void init_sensor(void (*init_func)(), const char *sensor_name) {
    Serial.printf("Initializing %s:\n", sensor_name);
    init_func();
    Serial.printf("- %s initialized\n", sensor_name);
#if MINIMOTE
    initted[sensor_name] = true;
#endif
}

void setup() {
    Serial.begin(115200);
#if WAIT_FOR_SERIAL
    while (!Serial);
    delay(2000);
#endif
#if RIVERMOTE
    Serial.println("Welcome to River Mote!");
#endif
#if MINIMOTE
    Serial.println("Welcome to Mini Mote (powered by River Mote)!");
#endif
    uint8_t mac[6] = {};
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    Serial.printf("device id: %02x%02x%02x%02x%02x%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    Serial.print("Initializing pmu:");
    if (!pmu_init()) {
		Serial.println("! pmu init failed! halting.");
		while (true) yield();
	}
	Serial.println("- pmu initialized");

    Serial.print("Initializing modem:");
	if (modem_init()) {
		Serial.println("- modem initialized");
	} else {
		Serial.println("! modem init failed!");
	}

#if RIVERMOTE
    // River mote-only init tasks
    rivermote_init();
#endif

    // Initialization of shared components (sensors)
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, I2C_FREQ);
    init_sensor(temp_init, "temperature sensor");
    init_sensor(turbidity_init, "turbidity sensor");
    init_sensor(tds_init, "TDS sensor");
    /*
    init_sensor(compass_init, "compass");
    init_sensor(imu_init, "IMU");
    */

    // Initialization of specific sensors that are only present on one of the mote types
#if RIVERMOTE
    init_sensor(spectral_init, "spectral sensor");
#endif
#if MINIMOTE
    init_sensor(env_init, "environmental sensors");
    init_sensor(ozone_init, "ozone sensor");
    init_sensor(velo_init, "air velocity sensor");
    init_sensor(pm_init, "particulate matter sensor");
    init_sensor(chamber_init, "chamber temp sensor");
    init_sensor(uv_init, "uv sensor");
#endif

#if RIVERMOTE
    // More river mote-only tasks that have to be done after sensors are initialized
    rivermote_post_sensor_init();
#endif
#if MINIMOTE
    // Mini mote-only tasks
    minimote_init(initted);
#endif

    Serial.println("Ready!");
}

void loop() {
#if RIVERMOTE
    rivermote_tick();
#endif
#if MINIMOTE
    minimote_tick();
#endif
    delay(10);
}
