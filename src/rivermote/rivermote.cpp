#include "rivermote/rivermote.h"

#if RIVERMOTE

#include <Arduino.h>

#include "rivermote/bluetooth.h"
#include "rivermote/control.h"
#include "rivermote/motors.h"
#include "sensors/rivermote/compass.h"
#include "sensors/rivermote/spectral.h"
#include "sensors/tds.h"
#include "sensors/temp.h"
#include "sensors/turbidity.h"
#include "modem.h"
#include "pmu.h"
#include "sd.h"

#define LOG_INTERVAL 1000 // ms between log entries
#define LOG_HEADER "Millis,Time,Batt%,BattV,Lat,Lng,Speed,Track,Heading,Temp,Turbidity,TDS,SpecViolet,SpecBlue,SpecGreen,SpecYellow,SpecOrange,SpecRed"

void rivermote_init() {
    Serial.println("enabling modem gps");
    if (modem_gps_enable()) {
        Serial.println("- modem gps enabled");
    } else {
        Serial.println("! modem gps enable failed!");
    }
}

void rivermote_post_sensor_init() {
    Serial.println("Initializing bluetooth:");
    if (bluetooth_init()) {
        Serial.println("- bluetooth initialized");
    } else {
        Serial.println("! bluetooth init failed!");
    }

    Serial.println("Initializing motors:");
    motors_init();
    control_init();
    Serial.println("- motors initialized");

    Serial.println("Initializing sd card:");
    if (sd_init()) {
        Serial.println("- sd initialized");
    } else {
        Serial.println("! sd init failed!");
    }
    sd_create_new(LOG_HEADER);
}

void rivermote_tick() {
    control_handle_input(bluetooth_get_pressed());
    control_autonomous_mode();

    static unsigned long lastLog = 0;
    if (millis() - lastLog > LOG_INTERVAL) {
        String time = modem_gps_read_time();
        ModemGPSData gps = modem_gps_read();
        float temperature = temp_read();
        float turbidity = get_turbidity();
        float tds = get_tds();
        SpectralData spectral = get_spectrum();
        sd_appendf("%lu,%s,%d,%.3f,%.6f,%.6f,%.2f,%.2f,%.2f,%.2f,%.4f,%.4f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f",
            millis(), time.c_str(), pmu_get_battery_percent(), pmu_get_battery_voltage(),
            gps.lat, gps.lng, gps.speed, gps.track, compass_read(),
            temperature, turbidity, tds,
            spectral.violet, spectral.blue, spectral.green, spectral.yellow, spectral.orange, spectral.red);
        bluetooth_printf("%.3fV %d%% %.0f%%, %s %s\n",
            pmu_get_battery_voltage(), pmu_get_battery_percent(),
            motors_get_max_speed() * 100, control_is_autonomous() ? "A" : "M", modem_gps_fixed() ? "FIX" : "NO FIX");
        lastLog = millis();
    }
}

#endif // RIVERMOTE
