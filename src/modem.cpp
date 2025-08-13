// https://edworks.co.kr/wp-content/uploads/2022/04/SIM7070_SIM7080_SIM7090-Series_AT-Command-Manual_V1.05.pdf

#include <HardwareSerial.h>
#include <string.h>
#include <TinyGsmClient.h>

#include "pins.h"

#include "modem.h"

#if LOG_MODEM
    #include <StreamDebugger.h>
    StreamDebugger debugger(Serial1, Serial);
    TinyGsm modem(debugger);
#else
    TinyGsm modem(Serial1);
#endif

static bool gpsEnabled, gpsFixed = false;

bool modem_init(unsigned long baud, uint max_retries) {
    // Initialize modem serial communication
    Serial1.begin(baud, SERIAL_8N1, PIN_MODEM_RX, PIN_MODEM_TX);
    pinMode(PIN_MODEM_PWR, OUTPUT);

    // Pulse modem power to reset it
    Serial.println("resetting modem");
    digitalWrite(PIN_MODEM_PWR, LOW);
    delay(100);
    digitalWrite(PIN_MODEM_PWR, HIGH);
    delay(1000);
    digitalWrite(PIN_MODEM_PWR, LOW);

    // Wait for the modem to start
    Serial.print("waiting for modem to start");
    uint retry = 0;
    while (!modem.testAT(1000) && retry < max_retries) {
        Serial.print(".");
        retry++;
    }
    Serial.println();
    return retry < max_retries;
}

bool modem_gps_enable() {
    gpsEnabled = modem.enableGPS();
    return gpsEnabled;
}

bool modem_gps_is_enabled() {
    return gpsEnabled;
}

bool modem_gps_disable() {
    if (gpsEnabled) {
        gpsEnabled = !modem.disableGPS();
    }
    return true; // Already disabled
}

ModemGPSData modem_gps_read() {
    // Get raw GPS data from module
    ModemGPSData data;
    String raw = modem.getGPSraw();
    // Parse returned data; see page 202 of the above datasheet for field info
    int parsed = sscanf(raw.c_str(), "%*[^,],%d,%*[^,],%f,%f,%*[^,],%f,%f", &gpsFixed, &data.lat, &data.lng, &data.speed, &data.track);
    if (parsed != 4) {
        data.lat = data.lng = data.speed = data.track = 0.f;
    }
    return data;
}

String modem_gps_read_time() {
    String raw = modem.getGPSraw();
    return raw.substring(4, 18); // Position 3 of raw field
}

bool modem_gps_fixed() {
    (void)modem_gps_read();
    return gpsFixed;
}
