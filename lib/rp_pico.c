#include <stdio.h>
#include <stdarg.h>
#include "hardware/resets.h"
#include "hardware/structs/i2c.h"
#include "pico/stdlib.h"
#include "ansi-esc.h"
#include "rp_pico.h"

/**
 * Prüft, ob ein I2C-Port (i2c0 oder i2c1) initialisiert und aktiv ist.
 */
bool is_i2c_initialized(i2c_inst_t *i2c) {
	// 1. Check: Ist der Block aus dem Reset-Zustand raus?
	uint reset_bit = (i2c == i2c0) ? RESETS_RESET_I2C0_BITS : RESETS_RESET_I2C1_BITS;
	bool not_in_reset = !(resets_hw->reset & reset_bit);

	if (!not_in_reset) return false;

	// 2. Check: Ist das Enable-Bit im Hardware-Register des Controllers gesetzt?
	// i2c_get_hw(i2c) gibt uns direkten Zugriff auf die Register-Struktur
	return (i2c_get_hw(i2c)->enable & I2C_IC_ENABLE_ENABLE_BITS) != 0;
}

/**
 * Reads a character from UART and parses escape sequences.
 * Non-blocking: returns KEY_NONE if no data is available.
 */
key_t get_key() {
	int c = getchar_timeout_us(0);             /* Get char without waiting */

	if (c == PICO_ERROR_TIMEOUT) return KEY_NONE; /* No input available */

	if (c == 13 || c == 10) {
		return KEY_ENTER;
	}

	if (c == 27) {                             /* Possible Escape Sequence */
		int next1 = getchar_timeout_us(1000);  /* Check for '[' */
		int next2 = getchar_timeout_us(1000);  /* Check for direction char */

		if (next1 == '[') {
			switch (next2) {
				case 'A': return KEY_UP;       /* ESC[A = Up */
				case 'B': return KEY_DOWN;     /* ESC[B = Down */
				case 'C': return KEY_RIGHT;    /* ESC[C = Right */
				case 'D': return KEY_LEFT;     /* ESC[D = Left */
			}
		}
		return KEY_ESC;                        /* Just the ESC key */
	}

	return (key_t)c;                           /* Return regular ASCII char */
}

void pico_log(log_level_t level, const char *fmt, ...){
	// 1. Get total milliseconds since boot
	uint32_t total_ms = to_ms_since_boot(get_absolute_time());

	// 2. Calculate time units
	uint32_t ms  = total_ms % 1000;          /* Remaining milliseconds */
	uint32_t s   = (total_ms / 1000) % 60;   /* Seconds */
	uint32_t m   = (total_ms / 60000) % 60;  /* Minutes */
	uint32_t h   = (total_ms / 3600000);     /* Hours */

	// 3. Print the timestamp (Format: 00:00:00:000)
	// %02u = 2 digits with leading zero, %03u = 3 digits
	printf(ANSI_DIM "[%02u:%02u:%02u:%03u] " ANSI_RESET, h, m, s, ms);

	// 4. Set color and prefix based on level
	switch (level) {
		case LOG_INFO:  printf(ANSI_GREEN  "[INFO]  " ANSI_RESET); break;
		case LOG_WARN:  printf(ANSI_YELLOW "[WARN]  " ANSI_RESET); break;
		case LOG_ERROR: printf(ANSI_RED    "[ERROR] " ANSI_RESET); break;
		case LOG_DEBUG: printf(ANSI_ITALIC ANSI_CYAN   "[DEBUG] " ANSI_RESET); break;
	}

	// 5. Process the actual message (like printf)
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);

	// 6. Always end with a newline and reset
	printf(ANSI_RESET "\n");
}
