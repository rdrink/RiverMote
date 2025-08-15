#include <esp_sleep.h>
#include <Wire.h>
#include <XPowersLib.h>

#include "pins.h"

XPowersPMU pmu;

// Configures PMU voltage and current limits.
static void pmu_configure_limits() {
    pmu.setVbusVoltageLimit(XPOWERS_AXP2101_VBUS_VOL_LIM_4V36); // 4.36V
    pmu.setVbusCurrentLimit(XPOWERS_AXP2101_VBUS_CUR_LIM_1500MA); // 1.5A
    pmu.setSysPowerDownVoltage(2600); // 2.6V
}

// Configures PMU power rails for peripherals.
static void pmu_configure_rails() {
    // If device has been soft reset, power cycle the modem to reset it
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED ) {
        pmu.disableDC3();
        delay(200);
    }
    // Modem main power rail
    pmu.setDC3Voltage(3000);
    pmu.enableDC3();
    // GPS antenna power rail
    pmu.setBLDO2Voltage(3300);
    pmu.enableBLDO2();
    // SD card power rail
    pmu.setALDO3Voltage(3300);
    pmu.enableALDO3();
}

// Configures PMU power measurement and battery monitoring.
static void pmu_configure_power_measurement() {
    pmu.enableBattDetection();
    pmu.enableVbusVoltageMeasure();
    pmu.enableBattVoltageMeasure();
    pmu.enableSystemVoltageMeasure();
    pmu.fuelGaugeControl(true, true); // Allow PMU to learn the battery curve and save to ROM
}

// Configures PMU charging parameters.
static void pmu_configure_charging() {
    // Disable TS pin detection to allow device to charge
    pmu.disableTSPinMeasure();
    // Set the precharge charging current
    pmu.setPrechargeCurr(XPOWERS_AXP2101_PRECHARGE_50MA); // 50mA
    // Set constant current charge current limit
    pmu.setChargerConstantCurr(XPOWERS_AXP2101_CHG_CUR_200MA); // 200mA
    // Set stop charging termination current
    pmu.setChargerTerminationCurr(XPOWERS_AXP2101_CHG_ITERM_25MA); // 25mA
    // Set charge cut-off voltage
    pmu.setChargeTargetVoltage(XPOWERS_AXP2101_CHG_VOL_4V1); // 4.1V
}

bool pmu_init() {
    // Initialize pmu
    if (!pmu.begin(Wire1, AXP2101_SLAVE_ADDRESS, PIN_PMU_SDA, PIN_PMU_SCL)) {
        return false;
    }
    // Configure PMU settings
    Serial.println("configuring limits");
    pmu_configure_limits();
    Serial.println("configuring rails");
    pmu_configure_rails();
    Serial.println("configuring power measurement");
    pmu_configure_power_measurement();
    Serial.println("configuring charging");
    pmu_configure_charging();
    // Set charge LED to be controlled by PMU
    pmu.setChargingLedMode(XPOWERS_CHG_LED_CTRL_CHG);
    return true;
}

int pmu_get_battery_percent() {
    if (!pmu.isBatteryConnect()) {
        return -1;
    }
    return pmu.getBatteryPercent();
}
