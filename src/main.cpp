#include <Arduino.h>
#include <Wire.h>

#include "sensors/imu.h"
#include "sensors/temp.h"
#include "modem.h"
#include "pins.h"
#include "pmu.h"
#include "sd.h"

#define WAIT_FOR_SERIAL 0
#define WAIT_FOR_GPS_FIX 1

void setup() {
    Serial.begin(115200);
#if WAIT_FOR_SERIAL
	while (!Serial);
	delay(1000);
#endif
	Serial.println("welcome to river mote!");

	// Initialize PMU
	Serial.println("initializing pmu");
    if (!pmu_init()) {
		Serial.println("pmu init failed! halting.");
		while (true) yield();
	}
	Serial.println("pmu initialized");

	// Initialize modem
	Serial.println("initializing modem");
	if (modem_init()) {
		Serial.println("modem initialized");
	} else {
		Serial.println("modem init failed!");
	}
	// Enable GPS and wait for fix
	Serial.println("enabling modem gps");
	if (modem_gps_enable()) {
#if WAIT_FOR_GPS_FIX
		Serial.print("modem gps enabled, waiting for fix");
		while (!modem_gps_fixed()) {
			Serial.print(".");
			delay(1000);
		}
#else
		Serial.println("modem gps enabled");
#endif
	} else {
		Serial.println("modem gps enable failed!");
	}

	// Initialize sensors
	Serial.println("initializing i2c bus");
	Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, I2C_FREQ);
	// Temperature sensor
	Serial.println("initializing temperature sensor");
	if (temp_init()) {
		Serial.println("temperature sensor initialized");
	} else {
		Serial.println("temperature sensor init failed!");
	}
	// IMU
	Serial.println("initializing imu");
	if (imu_init()) {
		Serial.println("imu initialized");
	} else {
		Serial.println("imu init failed!");
	}

	// Initialize SD card
	Serial.println("initializing sd card");
	if (sd_init()) {
		Serial.println("sd initialized");
	} else {
		Serial.println("sd init failed!");
	}

	Serial.println("ready.");
}

void loop() {
	
}
