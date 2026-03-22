#ifndef RP_PICO_H
#define RP_PICO_H
#include "hardware/i2c.h"
#include "i2c.h"
#include "ansi-esc.h"
#include "pico/stdio_usb.h"

#ifndef TESTING_ENABLED
#define TESTING_ENABLED 1
#endif

#ifndef INFO_ENABLED
#define INFO_ENABLED 1
#endif

#ifndef DEBUG_ENABLED
#define DEBUG_ENABLED 0
#endif

#ifndef LOG_NEW_LINE
#define LOG_NEW_LINE 1
#endif

#define LOG_PREFIX ANSI_COLOR_FN __func__ ANSI_RESET "():" ANSI_STR(__LINE__)

static inline void pico_stdio_init(void){
	stdio_init_all();

#if TESTING_ENABLED
	/* Block until USB is connected (development only) */
	while (!stdio_usb_connected()) {
		sleep_ms(100);
	}
#endif
}

/**
 * @brief Simple timestamp container.
 *
 * Values are derived from milliseconds since boot.
 */
typedef struct {
	uint32_t h;   /**< Hours since boot */
	uint32_t m;   /**< Minutes [0..59] */
	uint32_t s;   /**< Seconds [0..59] */
	uint32_t ms;  /**< Milliseconds [0..999] */
} pico_time_t;

/**
 * @brief Return current uptime as split timestamp fields.
 *
 * @return Timestamp struct containing hours, minutes, seconds and milliseconds.
 */
static inline pico_time_t pico_get_timestamp(void)
{
	uint32_t total_ms = to_ms_since_boot(get_absolute_time());
	uint32_t total_s  = total_ms / 1000;

	return (pico_time_t){
		.h  = total_s / 3600,
		.m  = (total_s / 60) % 60,
		.s  = total_s % 60,
		.ms = total_ms % 1000
	};
}

/**
 * @brief Print a formatted uptime timestamp.
 *
 * @details
 * Supported format tokens:
 * - h = hours (2 digits)
 * - m = minutes (2 digits)
 * - s = seconds (2 digits)
 * - S = milliseconds (3 digits)
 *
 * Any other character is printed unchanged.
 *
 * Example:
 * @code
 * pico_tsprintf("h:m:s");
 * pico_tsprintf("[h:m:s:S] ");
 * @endcode
 *
 * @param fmt Format string using h/m/s/S placeholders.
 */
static inline void pico_tsprintf(const char *fmt)
{
	if (!fmt) return;

	pico_time_t t = pico_get_timestamp();

	while (*fmt) {
		switch (*fmt) {
			case 'h': printf("%02u", t.h);  break;
			case 'm': printf("%02u", t.m);  break;
			case 's': printf("%02u", t.s);  break;
			case 'S': printf("%03u", t.ms); break;
			default:  putchar(*fmt);        break;
		}
		fmt++;
	}
}


/**
 * @brief Enumeration of supported keyboard inputs.
 * 
 * Includes standard ASCII characters and translated escape sequences 
 * for navigation and editing keys.
 */
typedef enum {
	KEY_NONE      = 0,   /**< No key pressed or timeout. */
	KEY_TAB       = 9,   /**< Tabulator key for autocomplete. */
	KEY_ENTER     = 13,  /**< Enter / Carriage Return key. */
	KEY_ESC       = 27,  /**< Escape key or start of ANSI sequence. */
	KEY_SPACE     = 32,  /**< Space bar. */

	/* Extended Keys (mapped from ANSI escape sequences) */
	KEY_UP        = 128, /**< Arrow Up - Previous history item. */
	KEY_DOWN      = 129, /**< Arrow Down - Next history item. */
	KEY_RIGHT     = 130, /**< Arrow Right - Move cursor right. */
	KEY_LEFT      = 131, /**< Arrow Left - Move cursor left. */
	KEY_BACKSPACE = 132, /**< Backspace - Delete character to the left. */
	KEY_DELETE    = 133, /**< Delete - Remove character at current position. */
	KEY_HOME      = 134, /**< Home - Jump to start of line. */
	KEY_END       = 135  /**< End - Jump to end of line. */
} key_t;

key_t get_key();

// Log Levels
typedef enum {
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR,
	LOG_DEBUG
} log_level_t;

// The core logging function
void pico_log(log_level_t level, const char *fmt, ...);

// Handy macros for shorter calls
// #define LOG_W(...) pico_log(LOG_WARN,  __VA_ARGS__)
//#define LOG_E(...) pico_log(LOG_ERROR, __VA_ARGS__)
#define LOG_E(fmt, ...) pico_log(LOG_WARN, ANSI_COLOR_FN "%s" ANSI_RESET "():%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define LOG_W(fmt, ...) pico_log(LOG_WARN, ANSI_COLOR_FN "%s" ANSI_RESET "():%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)

#if INFO_ENABLED
// #define LOG_I(...) pico_log(LOG_INFO, __VA_ARGS__)
#define LOG_I(fmt, ...) pico_log(LOG_INFO, ANSI_COLOR_FN "%s" ANSI_RESET "():%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)
#else
#define LOG_I(...) ((void)0) // Completely ignored by the compiler
#endif

// The Magic: If disabled, LOG_D does absolutely nothing
#if DEBUG_ENABLED
// #define LOG_D(...) pico_log(LOG_DEBUG, __VA_ARGS__)
#define LOG_D(fmt, ...) pico_log(LOG_DEBUG, ANSI_COLOR_FN "%s" ANSI_RESET "():%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)
#else
#define LOG_D(...) ((void)0) // Completely ignored by the compiler
#endif

#endif
