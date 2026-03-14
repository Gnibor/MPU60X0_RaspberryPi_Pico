/**
 * @file terminal.h
 * @brief Portable TUI Engine for Microcontrollers.
 */

#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>
#include <stdbool.h>
#include "rp_pico.h"

/** 
 * @name UI Geometry Definitions
 * @{ 
 */
#define TERM_WIDTH        130  /**< Total character width of the terminal. */
#define TERM_SIDEBAR_COL  90   /**< Starting column for the static telemetry sidebar. */
#define TERM_LOG_START    1    /**< First row of the scrolling log region. */
#define TERM_LOG_END      35   /**< Last row of the scrolling log region. */
#define TERM_CMD_ROW      36   /**< Row index for the persistent command-line prompt. */
#define TERM_MAX_HISTORY  10   /**< Number of commands to store in history. */
#define TERM_CMD_MAX_LEN  64   /**< Maximum length per command string. */
#define TERM_SIDEBAR_MAX  20   /**< Maximum number of dynamic sidebar items. */
/** @} */

/** @brief Global drawing lock to prevent overlapping output. */
extern volatile bool term_busy;

/* --- Log Macro Overrides --- */
#undef LOG_I
#undef LOG_W
#undef LOG_E

#define LOG_I(...) do { term_busy=true; term_scroll_log(); pico_log(LOG_INFO,  __VA_ARGS__); term_busy=false; } while(0)
#define LOG_W(...) do { term_busy=true; term_scroll_log(); pico_log(LOG_WARN,  __VA_ARGS__); term_busy=false; } while(0)
#define LOG_E(...) do { term_busy=true; term_scroll_log(); pico_log(LOG_ERROR, __VA_ARGS__); term_busy=false; } while(0)

/** @brief Supported data types for sidebar telemetry items. */
typedef enum { 
    TYPE_INT,    /**< 16-bit signed integer (int16_t). */
    TYPE_FLOAT,  /**< 32-bit floating point. */
    TYPE_STRING  /**< Null-terminated character string. */
} data_type_t;

/** @brief Structure defining a single telemetry entry in the sidebar. */
typedef struct {
    const char* label;
    void* value_ptr;
    data_type_t type;
    const char* unit;
} sidebar_item_t;

/** @brief Helper structure for the "dictionary" of watchable variables. */
typedef struct {
    const char* name;
    void* ptr;
    data_type_t type;
    const char* unit;
} watchable_t;

/** @brief Structure defining a shell command. */
typedef struct {
    const char* name;
    void (*func)(const char* args);
    const char* help;
    const char* usage;
} command_t;

/* --- Public API --- */

/** @brief Links hardware variables to the UI configuration. */
void term_cfg_init(void);

/** @brief Initializes screen, scroll regions and UI frame. */
void term_init(void);

/** @brief Periodically refreshes the dynamic sidebar. */
void term_update_sidebar(void);

/** @brief Prepares cursor for log entry within scroll region. */
void term_scroll_log(void);

/** @brief Non-blocking UART input handler. */
void term_handle_input(key_t key);

/** @brief Prints the system banner from CMake metadata. */
void term_print_banner(void);

/** @brief Searches for key=value pairs in the command arguments. */
const char* term_get_arg(const char* args, const char* key);

/** @brief Registers a variable to be displayed in the sidebar at runtime. */
bool term_sidebar_register(const char* label, void* ptr, data_type_t type, const char* unit);

bool term_sidebar_remove(const char* label);
#endif // TERMINAL_H
