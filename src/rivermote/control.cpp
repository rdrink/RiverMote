#include "rivermote/control.h"

#if RIVERMOTE

#include <PID_v1.h>

#include "rivermote/bluetooth.h"
#include "rivermote/motors.h"
#include "rivermote/nav.h"
#include "sensors/rivermote/compass.h"
#include "modem.h"

// Button state tracking
static bool button1Last = false; // Decrease speed
static bool button2Last = false; // Increase speed
static bool button3Last = false; // Enable/disable autonomous mode

static bool autonomous = false; // Autonomous mode flag
// PID control variables for heading control
static double heading = 0.0, turnDifferential = 0.0, targetHeading = 0.0;
static PID headingPID(&heading, &turnDifferential, &targetHeading, 1.5, 0.0, 0.3, AUTOMATIC);

// Waypoints for autonomous navigation
static Waypoint waypoints[] = {
	{ 41.90643725287359, -87.65172064304353 },
    { 41.90640132130173, -87.65193521976471 },
	{ 41.90638535170774, -87.65182793140413 },
	{ 41.90640930609723, -87.65172064304353 },
	{ 41.90643725287359, -87.65172064304353 },
};
static uint currentWaypoint = 0; // Current waypoint index
#define NUM_WAYPOINTS (sizeof(waypoints) / sizeof(waypoints[0]))
#define DISTANCE_THRESHOLD 5.0 // Distance threshold to consider waypoint reached (in meters)

/**
 * Helper to handle button press events.
 * @param buttons current button bitfield
 * @param mask button mask to check
 * @param last reference to last button state
 * @param action function to call on button press
 */
static void on_button_press(uint8_t buttons, BluetoothButton mask, bool last, void (*action)()) {
    bool pressed = (buttons & mask) != 0;
    if (pressed && !last) {
        action();
    }
    last = pressed;
}

void control_init() {
	headingPID.SetMode(AUTOMATIC);
	headingPID.SetOutputLimits(-255, 255);
	headingPID.SetSampleTime(100);
}

void control_handle_input(uint8_t buttons) {
    // If being manually controlled--set motor direction based on buttons pressed
	if (!autonomous) {
		if (buttons & BTN_UP) { // up
			motors_set(255, 255);
		} else if (buttons & BTN_DOWN) { // down
			motors_set(-255, -255);
		} else if (buttons & BTN_LEFT) { // left
			motors_set(-255, 255);
		} else if (buttons & BTN_RIGHT) { // right
			motors_set(255, -255);
		} else {
			motors_set(0, 0);
		}
	}

	// Set motor max speed based on buttons 1 and 2
	on_button_press(buttons, BTN_1, button1Last, [](){
		motors_set_max_speed(motors_get_max_speed() - 0.1f); // Decrease speed
	});
	on_button_press(buttons, BTN_2, button2Last, [](){
		motors_set_max_speed(motors_get_max_speed() + 0.1f); // Increase speed
	});
	// Toggle autonomous mode based on button 3
	on_button_press(buttons, BTN_3, button3Last, [](){
		autonomous = !autonomous; // Toggle autonomous mode
		if (autonomous) {
			// Reset PID states
			headingPID.SetMode(MANUAL);
			headingPID.SetMode(AUTOMATIC);
		}
	});
}

void control_autonomous_mode() {
	if (!autonomous) {
		return; // Disabled
	}
	if (!modem_gps_is_enabled()) {
		return; // GPS is required to navigate
	}

	// Get GPS and compass data
	ModemGPSData gps = modem_gps_read();
    if (gps.lat == 0.f || gps.lng == 0.f) {
        return; // Invalid fix
	}
	heading = compass_read();

	// Calculate heading and distance to current waypoint
	Waypoint target = waypoints[currentWaypoint];
	double bearing = calculate_bearing(gps.lat, gps.lng, target.lat, target.lng);
	double diff = fmod(bearing - heading + 540.0, 360.0) - 180.0;
	targetHeading = heading + diff;
	double distance = calculate_distance(gps.lat, gps.lng, target.lat, target.lng);

	// Update PID controller and set differential motor speeds based on output
	headingPID.Compute();
	motors_set(255 - turnDifferential, 255 + turnDifferential);

	// Check if waypoint is reached, and move to next if so
	if (distance < DISTANCE_THRESHOLD) {
        currentWaypoint++;
		// Stop and exit autonomous if all waypoints have been reached
        if (currentWaypoint >= NUM_WAYPOINTS) {
            motors_set(0, 0);
			autonomous = false;
		}
    }
}

bool control_is_autonomous() {
	return autonomous;
}

#endif // RIVERMOTE
