#include <Arduino.h>

#include "modem.h"
#include "sd.h"
#include "pmu.h"

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
	if (!modem_init()) {
		Serial.println("modem init failed!");
		return;
	}
	Serial.println("modem initialized");
	// Enable GPS and wait for fix
	Serial.println("enabling modem gps");
	if (!modem_gps_enable()) {
		Serial.println("modem gps enable failed!");
	}
#if WAIT_FOR_GPS_FIX
	Serial.print("modem gps enabled, waiting for fix");
	while (!modem_gps_fixed()) {
		Serial.print(".");
		delay(1000);
	}
#else
	Serial.println("modem gps enabled");
#endif

	// Initialize SD card
	Serial.println("initializing sd card");
	if (!sd_init()) {
		Serial.println("sd init failed!");
	}
	Serial.println("sd initialized");

	Serial.println("ready.");
}

void loop() {
	
}
