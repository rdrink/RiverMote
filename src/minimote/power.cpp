#include "minimote/power.h"

#if MINIMOTE

#include <Arduino.h>
#include <esp_timer.h>
#include <esp_sleep.h>
#include <time.h>
#include <Wire.h>

#include "minimote/comm.h"
#include "sensors/minimote/air_velocity.h"
#include "sensors/minimote/chamber.h"
#include "sensors/minimote/env.h"
#include "sensors/minimote/ozone.h"
#include "sensors/minimote/pm.h"
#include "sensors/temp.h"
#include "modem.h"
#include "pmu.h"

#define LOW_BATTERY_VOLT 3.65f // Low power mode will be entered under this voltage
#define LOW_BATTERY_SLEEP_SEC 21600 // Time to deep sleep when low battery is detected, in seconds (6 hours)

#define SAMPLE_SLOT_SECONDS 180 // Duration to sleep between samples, in seconds (3 minutes). Should be an even divisor of PUBLISH_SLOT_SECONDS
#define PUBLISH_SLOT_SECONDS 900 // Duration to sleep between publish windows, in seconds (15 minutes). Can be changed by sending a control message
#define SLOT_WINDOW_PERCENT 0.1f // Percentage after the slot duration to consider "still within the window"

#define DELAY_AFTER_WAKE_S 2 // Time to delay after waking up to allow sensors to power on before we try to communicate with them
#define DELAY_AFTER_I2C_S 8 // Time to delay after initializing i2c before trying to communicate with sensors, to prevent i2c errors
// Unfortunately this delay has to be very long due to the environmental sensor taking a long time to read out proper values

#define SLEEP_LOOP_US 1000000 // Duration of time in light sleep mode before waking up to call `sleep_loop()`, in microseconds (1 second)

static uint32_t sampleSlot = SAMPLE_SLOT_SECONDS, pubSlot = PUBLISH_SLOT_SECONDS;

// Loop function to call periodically (every `SLEEP_LOOP_US`) while in light sleep.
static inline void sleep_loop() {
    pm_loop();
}

// Reinitializes i2c and all sensors connected to it.
static void reinit_i2c() {
    env_init();
    ozone_init();
    velo_init();
    chamber_init();
    temp_init();
}

/**
 * Puts the device to sleep for the specified duration.
 * @note If deep is specified, all peripherals will be powered down and the device will be reset upon wakeup.
 */
static void minimote_enter_sleep(uint32_t sleep_sec, bool deep) {
    Serial.printf("%s sleeping for %lu sec\n", deep ? "deep" : "light", (unsigned long)sleep_sec);
    uint64_t sleepUs = (uint64_t)sleep_sec * 1000000ULL;

    pmu_set_sensor_power(false);

    if (deep) {
        minimote_comm_deinit();
        modem_deinit(); // Disconnect from cell + fully power down modem
        pmu_set_modem_power(false);
        esp_sleep_enable_timer_wakeup(sleepUs);
        esp_deep_sleep_start();
        // Deep sleep will reset the device, so code execution will not continue past esp_deep_sleep_start()
        // It will pick back up at setup() after waking up
        return;
    }

    // Light sleep path; put modem to sleep and begin our sleep cycle
    modem_set_sleep(true);
    Serial.end();
    // Wake up periodically to call our loop function, and continue this until the sleep duration has fully elapsed
    uint64_t endUs = esp_timer_get_time() + sleepUs;
    while (esp_timer_get_time() < endUs) {
        esp_sleep_enable_timer_wakeup(SLEEP_LOOP_US);
        esp_light_sleep_start();
        sleep_loop();
    }
    Serial.begin(115200);

    // Upon waking up, restore sensor power and wake modem
    pmu_set_sensor_power(true);
    // Give sensors a moment to power up before trying to initialize them
    delay(DELAY_AFTER_WAKE_S * 1000);
    // Reconnect to i2c sensors
    reinit_i2c();
    // Give sensors another moment to get ready before we return to the main loop and take readings
    delay(DELAY_AFTER_I2C_S * 1000);
}

void minimote_manage_power() {
    float battV = pmu_get_battery_voltage();
    // If battery voltage is very low, power down as much as we can to conserve energy
    if (battV > 0.f && battV < LOW_BATTERY_VOLT) {
        Serial.printf("battery low (%.3fV), entering long sleep\n", battV);
        minimote_enter_sleep(LOW_BATTERY_SLEEP_SEC, true);
    }

    // Otherwise, calculate how long to sleep until the next sample window
    time_t now = time(nullptr);
    if (now > 0) {
        uint32_t offset = (uint32_t)(now % sampleSlot);
        // If we are within the trailing edge of the window (i.e. we just published and are still
        // inside it), sleep a full slot so we don't immediately re enter the window on the next wake
        uint32_t window = (uint32_t)(sampleSlot * SLOT_WINDOW_PERCENT);
        uint32_t sleepSec = (offset < window) ? sampleSlot : (sampleSlot - offset);
        minimote_enter_sleep(sleepSec, false);
    } else {
        // Invalid time, fallback to a reasonable sleep duration
        minimote_enter_sleep(PUBLISH_SLOT_SECONDS, false);
    }
}

bool minimote_within_publish_window() {
    time_t now = time(nullptr);
    if (now <= 0) {
        // If time is invalid, assume we should publish (better to publish too often than miss data)
        return true;
    }
    uint32_t offset = (uint32_t)(now % pubSlot);
    uint32_t window = (uint32_t)(pubSlot * SLOT_WINDOW_PERCENT);
    // One-sided window: only publish after the boundary, not before
    // This prevents publishing early on the sample before the publish boundary
    return offset < window;
}

void minimote_set_slot_seconds(uint32_t sample, uint32_t publish) {
    if (sample > 0) {
        sampleSlot = sample;
    }
    if (publish > 0) {
        pubSlot = publish;
    }
}

#endif // MINIMOTE
