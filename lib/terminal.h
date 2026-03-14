/**
 * @file terminal.h
 * @brief Portable TUI Engine for Microcontrollers.
 * 
 * This engine provides a structured terminal layout with a scrolling log area,
 * a static sidebar for live telemetry, and a persistent command-line interface.
 * Designed for ANSI-compatible serial terminals (e.g., Neovim :term, Minicom, PuTTY).
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
#define TERM_WIDTH       130  /**< Total character width of the terminal. */
#define TERM_SIDEBAR_COL 90   /**< Starting column for the static telemetry sidebar. */
#define TERM_LOG_START   1    /**< First row of the scrolling log region. */
#define TERM_LOG_END     35   /**< Last row of the scrolling log region. */
#define TERM_CMD_ROW     36   /**< Row index for the persistent command-line prompt. */
#define TERM_MAX_HISTORY  10  /**< Number of commands to store in history. */
#define TERM_CMD_MAX_LEN  64  /**< Maximum length per command string. */
/** @} */

/** 
 * @brief Global drawing lock. 
 * Set to true when the engine is actively writing ANSI sequences to prevent 
 * overlapping output from different functions or cores.
 */
extern volatile bool term_busy;

/* --- Log Macro Overrides --- */

#undef LOG_I
#undef LOG_W
#undef LOG_E
#undef DEBUG_ENABLED

/** @brief Global debug toggle. Set to 1 to enable LOG_D output. */
#define DEBUG_ENABLED 0

/**
 * @brief Thread-safe INFO log macro.
 * Locks the UI, prepares the scroll region, prints the log, and unlocks.
 */
#define LOG_I(...) do { term_busy=true; term_scroll_log(); pico_log(LOG_INFO,  __VA_ARGS__); term_busy=false; } while(0)

/**
 * @brief Thread-safe WARN log macro.
 * Uses yellow formatting for non-critical warnings.
 */
#define LOG_W(...) do { term_busy=true; term_scroll_log(); pico_log(LOG_WARN,  __VA_ARGS__); term_busy=false; } while(0)

/**
 * @brief Thread-safe ERROR log macro.
 * Uses red formatting for critical system errors.
 */
#define LOG_E(...) do { term_busy=true; term_scroll_log(); pico_log(LOG_ERROR, __VA_ARGS__); term_busy=false; } while(0)

/**
 * @brief Supported data types for sidebar telemetry items.
 */
typedef enum { 
	TYPE_INT,    /**< 16-bit signed integer (int16_t). */
	TYPE_FLOAT,  /**< 32-bit floating point. */
	TYPE_STRING  /**< Null-terminated character string. */
} data_type_t;

/**
 * @brief Structure defining a single telemetry entry in the sidebar.
 */
typedef struct {
	const char* label;     /**< Label displayed on the left (e.g., "Accel X"). */
	void* value_ptr;       /**< Pointer to the actual data variable. */
	data_type_t type;      /**< Data type for correct casting and printing. */
	const char* unit;      /**< Measurement unit (e.g., "g", "raw", "°C"). */
} sidebar_item_t;

/**
 * @brief Structure defining a shell command.
 */
typedef struct {
	const char* name;                /**< The string the user types (e.g., "help"). */
	void (*func)(const char* args);  /**< Function pointer to the command's implementation. */
	const char* help;                /**< Short description for the @ref cmd_help_auto function. */
	const char* usage;               /**< Example: "device=on/off samples=N" */
} command_t;

/* --- Public API --- */

/**
 * @brief Links hardware variables to the sidebar configuration.
 * @note Must be called after @ref mpu_init and before @ref term_init.
 */
void term_cfg_init(void);

/**
 * @brief Clears the screen, sets up scroll regions, and draws the static UI frame.
 */
void term_init(void);

/**
 * @brief Refreshes all values in the telemetry sidebar.
 * @note Call this periodically (e.g., 50Hz) in your main loop.
 */
void term_update_sidebar(void);

/**
 * @brief Prepares the cursor for a log entry within the scroll region.
 */
void term_scroll_log(void);

/**
 * @brief Non-blocking handler for UART/USB input.
 * Processes keystrokes, manages the command buffer, and executes commands.
 * @param key The character received from @ref get_key.
 */
void term_handle_input(key_t key);

void term_print_banner(void);
const char* term_get_arg(const char* args, const char* key);
/**
 * @brief Wrapper for the core @ref pico_log function.
 * Handles ANSI positioning for standard logging calls.
 * @param level Severity level (INFO/WARN/ERROR/DEBUG).
 * @param fmt Printf-style format string.
 * @param ... Additional arguments for format string.
 */
void term_log_wrapper(log_level_t level, const char* fmt, ...);

#endif // TERMINAL_H
