#include "minimote/comm.h"

#if MINIMOTE

#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_mac.h>
#include <PubSubClient.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "minimote/ota.h"
#include "minimote/power.h"
#include "modem.h"
#if __has_include("secrets.h")
    #include "secrets.h"
#endif

// The below configuration options can be overridden by defining them in secrets.h or elsewhere.
#ifndef MQTT_BROKER
    #define MQTT_BROKER ""
#endif
#ifndef MQTT_PORT
    #define MQTT_PORT 1883
#endif
#ifndef MQTT_PASSWORD
    #define MQTT_PASSWORD ""
#endif
#define MAX_MQTT_PAYLOAD_SIZE 1024

static PubSubClient mqtt(MQTT_BROKER, MQTT_PORT, modemClient);
static char endpoint[32], user[64];
static char topicInit[64], topicData[64], topicControl[64], topicAck[64];

static bool ensure_mqtt() {
    if (modem_client_is_connected()) {
        return true;
    }
    Serial.println("mqtt disconnected, attempting reconnect");
    mqtt.disconnect();
    if (!mqtt.connect(endpoint, user, MQTT_PASSWORD)) {
        Serial.println("mqtt connect failed!");
        return false;
    }
    return true;
}

static void handle_control(char *topic, byte *payload, unsigned int len) {
    if (strcasecmp(topic, topicControl) != 0) {
        return; // Not a control message
    }
    if (payload == nullptr || len == 0) {
        return; // Empty message
    }
    // Parse payload and extract command
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload, len);
    if (err || !doc["cmd"].is<const char*>()) {
        Serial.printf("invalid control message json: %s\n", err.c_str());
        return;
    }
    const char *cmd = doc["cmd"];
    Serial.printf("received control command: %s\n", cmd);
    
    // Acknowledge we received the command
    char ackPayload[192];
    snprintf(ackPayload, sizeof(ackPayload), "{\"ok\":true,\"cmd\":\"%s\"}", cmd);
    mqtt.publish(topicAck, ackPayload);
    delay(500); // Ensure ack is sent
    
    // Execute the command
    if (strcasecmp(cmd, "reboot") == 0) {
        ESP.restart();
    } else if (strcasecmp(cmd, "ota") == 0) {
        // Deinit MQTT so we can connect to HTTP
        minimote_comm_deinit();
        // Disable modem PSM to ensure it doesn't interfere with the OTA process
        modem_set_psm(false);
        if (!ota_do_update(doc["server"].as<const char*>(), doc["path"].as<const char*>())) {
            Serial.println("OTA update failed");
            // OTA failed and we will be returning to this current code, so re-enable PSM to save power
            modem_set_psm(true);
            return;
        }
        delay(1000);
        ESP.restart();
    } else if (strcasecmp(cmd, "set_slot") == 0) {
        if (doc["sample"].is<int>()) {
            minimote_set_slot_seconds(doc["sample"].as<uint32_t>(), 0);
        }
        if (doc["publish"].is<int>()) {
            minimote_set_slot_seconds(0, doc["publish"].as<uint32_t>());
        }
    } else {
        Serial.println("unknown control command");
        return;
    }
}

bool minimote_comm_init() {
    // Create a device endpoint based off the device's unique MAC address, which will be used to generate a unique
    // client ID, username, and topics for this device
    uint8_t mac[6] = {};
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(endpoint, sizeof(endpoint), "%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    snprintf(user, sizeof(user), "minimote-%s", endpoint);
    snprintf(topicInit, sizeof(topicInit), "minimote/%s/init", endpoint);
    snprintf(topicData, sizeof(topicData), "minimote/%s/data", endpoint);
    snprintf(topicControl, sizeof(topicControl), "minimote/%s/control", endpoint);
    snprintf(topicAck, sizeof(topicAck), "minimote/%s/ack", endpoint);
    Serial.printf("device endpoint=%s user=%s topicInit=%s topicData=%s topicControl=%s topicAck=%s\n", endpoint, user, topicInit, topicData, topicControl, topicAck);
    // Connect to MQTT and subscribe to control topic
    mqtt.setKeepAlive(60);
    mqtt.setCallback(handle_control);
    return mqtt.setBufferSize(MAX_MQTT_PAYLOAD_SIZE);
}

void minimote_comm_deinit() {
    mqtt.disconnect();
}

bool minimote_comm_sync_time() {
    time_t epoch = modem_cell_read_time();
    if (epoch <= 0) {
        Serial.println("failed to sync time");
        return false;
    } else if (epoch < 1700000000) {
        Serial.printf("time synced, but seems wrong? (%ld)\n", epoch);
        return false;
    }

    // Syncronize system clock with NTP time
    timeval tv = { .tv_sec = epoch, .tv_usec = 0 };
    settimeofday(&tv, nullptr);
    Serial.printf("time synced, epoch=%ld\n", epoch);
    return true;
}

void minimote_comm_control_window(uint32_t window) {
    if (!ensure_mqtt()) {
        return;
    }
    // Subscribe to receive control messages
    if (!mqtt.subscribe(topicControl, 1)) {
        Serial.println("failed to subscribe control topic");
        return;
    }

    // Run MQTT loop for the duration of the control window to allow for incoming messages to be processed
    unsigned long start = millis();
    while (millis() - start < window) {
        mqtt.loop();
    }

    // Now done, unsubscribe as we will be sleeping shortly and cannot receive messages while asleep
    if (!mqtt.unsubscribe(topicControl)) {
        Serial.println("failed to unsubscribe control topic");
        return;
    }
}

bool minimote_comm_publish_sample(const char *sample) {
    if (!ensure_mqtt()) {
        return false;
    }
    bool published = mqtt.publish(topicData, sample);
    Serial.printf("sample publish %s\n", published ? "ok" : "failed");
    return published;
}

bool minimote_comm_publish_init(const char *init) {
    if (!ensure_mqtt()) {
        return false;
    }
    bool published = mqtt.publish(topicInit, init);
    Serial.printf("init publish %s\n", published ? "ok" : "failed");
    return published;
}

#endif // MINIMOTE
