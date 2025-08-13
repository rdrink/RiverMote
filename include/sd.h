#pragma once

/**
 * Initialize communication with the SD card.
 * @return true on success
 */
bool sd_init();

/**
 * Create a new log file and write an optional header.
 * @param filename name of the file to create
 * @param header header row to write, or `nullptr` for no header
 * @return true on success
 */
bool sd_create_new(const String &filename, const char *header = nullptr);

/**
 * @return true if SD initialized and log file open ready
 */
bool sd_is_ready();

/**
 * Append a line to the currently open log file.
 * @param line line to write
 * @return true on success
 * @note No newline is needed, it is added automatically.
 */
bool sd_append(const char *line);

/**
 * Append a CSV line using printf-style formatting.
 * @return true on success
 */
bool sd_appendf(const char *fmt, ...);

