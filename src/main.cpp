#include <Arduino.h>
#include <WiFi.h>

#define WIFI_SSID "doUntil"
#define WIFI_PASS "saf3@hom3"

#define PIN_AMBI 36 // A4
#define PIN_PHOTORESISTOR 39 // A3

void setup() {
	Serial.begin(115200);

	pinMode(PIN_AMBI, INPUT);
	pinMode(PIN_PHOTORESISTOR, INPUT);

	WiFi.mode(WIFI_STA);
	Serial.printf("connecting to wifi with SSID %s and pass %s\n", WIFI_SSID, WIFI_PASS);
	WiFi.begin(WIFI_SSID, WIFI_PASS);
	while (WiFi.status() != WL_CONNECTED);
	Serial.print("connected to wifi, IP address: ");
	Serial.println(WiFi.localIP());
}

void loop() {
	uint16_t ambi = analogRead(PIN_AMBI);
	uint16_t photores = analogRead(PIN_PHOTORESISTOR);
    Serial.printf("ambi: %d, photores: %d\n", ambi, photores);
	delay(1000);
}
