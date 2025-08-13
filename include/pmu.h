#pragma once

/**
 * Initialize the PMU (power management unit).
 * @return true if initialization was successful
 */
bool pmu_init();

/**
 * Get the current battery percentage.
 * @return battery percentage (0-100) or -1 if battery is not connected
 */
int pmu_get_battery_percent();
