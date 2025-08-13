#include <FS.h>
#include <stdarg.h>
#include <SD_MMC.h>

#include "pins.h"

#include "sd.h"

#define FORMAT_BUFFER_SIZE 512 // Buffer size for sd_appendf

static File logFile;
static bool sdReady = false; // True if initialized and log file open

bool sd_init() {
    SD_MMC.setPins(PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0);
    return SD_MMC.begin("/sdcard", true);
}

bool sd_create_new(const String &filename, const char *header) {
    logFile = SD_MMC.open(filename, FILE_WRITE);
    if (!logFile) {
        return false;
    }
    Serial.printf("logging to %s\n", filename);

    // Write header if applicable
    if (header) {
        logFile.println(header);
        logFile.flush();
    }
    sdReady = true;
    return true;
}

bool sd_is_ready() {
    return sdReady;
}

bool sd_append(const char *line) {
    if (!sdReady || !logFile) {
        return false;
    }
    if (logFile.println(line)) {
        logFile.flush();
        return true;
    }
    return false;
}

bool sd_appendf(const char *fmt, ...) {
    char buf[FORMAT_BUFFER_SIZE];
    va_list ap; va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    return sd_append(buf);
}
