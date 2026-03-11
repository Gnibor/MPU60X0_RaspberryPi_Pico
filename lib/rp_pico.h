#ifndef RP_PICO_H
#define RP_PICO_H
#include "hardware/i2c.h"
#define DEBUG_ENABLED 0

/* Enum for readable key codes */
typedef enum {
    KEY_NONE  = 0,      /* No key pressed */
    KEY_UP    = 1,      /* Arrow Up key pressed */
    KEY_DOWN  = 2,      /* Arrow Down key pressed */
    KEY_RIGHT = 3,      /* Arrow Right key pressed */
    KEY_LEFT  = 4,      /* Arrow Left key pressed */
    KEY_ENTER = 13,     /* Enter / Carriage Return */
    KEY_ESC   = 27      /* Escape key */
} key_t;

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
#define LOG_I(...) pico_log(LOG_INFO,  __VA_ARGS__)
#define LOG_W(...) pico_log(LOG_WARN,  __VA_ARGS__)
#define LOG_E(...) pico_log(LOG_ERROR, __VA_ARGS__)

// The Magic: If disabled, LOG_D does absolutely nothing
#if DEBUG_ENABLED
    #define LOG_D(...) pico_log(LOG_DEBUG, __VA_ARGS__)
#else
    #define LOG_D(...) ((void)0) // Completely ignored by the compiler
#endif

bool is_i2c_initialized(i2c_inst_t *i2c);
#endif
