/**
 * @file terminal_cfg.c
 * @brief Application-specific configuration for the TUI.
 * 
 * This file acts as the "glue" between the hardware abstraction layer (MPU60X0)
 * and the generic terminal engine. It defines which data fields are displayed
 * in the sidebar and which functions are mapped to shell commands.
 */

#include <stdio.h>
#include "terminal.h"
#include "ansi-esc.h"
#include "mpu60x0.h"
#include "stdlib.h"
#include "string.h"

/** @brief External reference to the automatic help generator in terminal.c. */
extern void cmd_help_auto(const char* args);

/**
 * @brief Configuration array for the right-hand sidebar.
 * 
 * Each entry links a label to a data pointer. 
 * Note: value_ptrs are initialized to NULL and linked at runtime in @ref term_cfg_init.
 */
sidebar_item_t sidebar_config[] = {
    {"Accel X", NULL, TYPE_FLOAT,   "g"},
    {"Accel Y", NULL, TYPE_FLOAT,   "g"},
    {"Accel Z", NULL, TYPE_FLOAT,   "g"},
    {"Temp   ", NULL, TYPE_FLOAT,   "C"},
    {"Gyro X",  NULL, TYPE_FLOAT,   "°/s"},
    {"Gyro Y",  NULL, TYPE_FLOAT,   "°/s"},
    {"Gyro Z",  NULL, TYPE_FLOAT,   "°/s"},
    {"Status ", "ACTIVE", TYPE_STRING, ""} /**< Static system status string. */
};

/** @brief Total number of registered sidebar items. */
const int sidebar_count = sizeof(sidebar_config) / sizeof(sidebar_item_t);

/**
 * @brief Link hardware structure members to the UI sidebar configuration.
 * 
 * This function resolves the memory addresses of the scaled sensor data
 * within the global @ref g_mpu structure.
 */
void term_cfg_init() {
    /* Linking Accelerometer scaled values (g) */
    sidebar_config[0].value_ptr = (void*)&(g_mpu->v.accel.g.x);
    sidebar_config[1].value_ptr = (void*)&(g_mpu->v.accel.g.y);
    sidebar_config[2].value_ptr = (void*)&(g_mpu->v.accel.g.z);
    
    /* Linking Temperature (Celsius) */
    sidebar_config[3].value_ptr = (void*)&(g_mpu->v.temp.celsius);
    
    /* Linking Gyroscope scaled values (dps - degrees per second) */
    sidebar_config[4].value_ptr = (void*)&(g_mpu->v.gyro.dps.x);
    sidebar_config[5].value_ptr = (void*)&(g_mpu->v.gyro.dps.y);
    sidebar_config[6].value_ptr = (void*)&(g_mpu->v.gyro.dps.z);
}

/* --- Command Implementations --- */

/** @brief Command: Show MPU connection status. */
void cmd_mpu_status(const char* a){ (void)a; LOG_I("MPU6050 is %sONLINE%s", ANSI_GREEN, ANSI_RESET); }

/**
 * @brief Command: calib [samples]
 * Usage examples: "calib" (uses default) or "calib 100"
 */
void cmd_mpu_calibrate(const char* a) {
    uint8_t samples = 50; // Default fallback

    /* If arguments were provided, convert string to integer */
    if (a != NULL) {
        samples = (uint8_t)atoi(a);
    }

    LOG_I("Starting calibration with %u samples...", samples);
    mpu_calibrate((MPU_ACCEL_Z | MPU_GYRO), samples);
}

/** @brief Command: Perform full hardware reset via register 0x6B. */
void cmd_reset(const char* a){ (void)a; mpu_reset(MPU_RESET_ALL); LOG_W("System Reset triggered!"); }

/** @brief Command: Force MPU into low-power sleep mode. */
void cmd_sleep(const char* a){
	if (a == NULL) {
		LOG_W("Usage: sleep device=(on/off) / temp=(on/off)");
		return;
	}

	/* Check for the specific strings */
	const char* device = term_get_arg(a, "device");
	const char* temp = term_get_arg(a, "temp");
	if (strcmp(device, "on") == 0){
		mpu_sleep(MPU_SLEEP_DEVICE_ON);
		LOG_I("Sleep mode enabled (Device ON).");
	}else if (strcmp(device, "off") == 0) {
		mpu_sleep(MPU_SLEEP_ALL_OFF);
		LOG_I("Sleep mode disabled (Device OFF).");
	}else if(strcmp(temp, "on") == 0){
		mpu_sleep(MPU_SLEEP_TEMP_ON);
		LOG_I("Sleep mode enabled (Temp ON).");
	}else if (strcmp(temp, "off") == 0) {
		mpu_sleep(MPU_SLEEP_TEMP_OFF);
		LOG_I("Sleep mode disabled (Temp OFF).");
	}else{
		LOG_E("Invalid argument: '%s'. Use 'device=(on/off)' or 'temp=(on/off)'.", a);
	}

	LOG_I("Sleep mode enabled.");
}

/** @brief Command: clear -> Cleans the log area (Row 1 to 35) */
void cmd_clear_logs(const char* a) {
    (void)a;
    term_busy = true;
    // Loop through log rows and clear each one
    for (int i = TERM_LOG_START; i <= TERM_LOG_END; i++) {
        printf("\033[%d;1H\033[K", i);
    }
    // Refresh the vertical separator that might have been hit
    for (int i = 1; i < TERM_CMD_ROW; i++) {
        printf("\033[%d;%dH|", i, TERM_SIDEBAR_COL - 2);
    }
    LOG_I("Logs cleared.");
    term_busy = false;
}

void cmd_uptime(const char* a) {
    (void)a; // Silence unused parameter warning

    // Get total time since boot in milliseconds
    uint32_t total_ms = to_ms_since_boot(get_absolute_time());

    // Calculate units
    uint32_t ms = total_ms % 1000;
    uint32_t s  = (total_ms / 1000) % 60;
    uint32_t m  = (total_ms / 60000) % 60;
    uint32_t h  = (total_ms / 3600000);

    // Format: 00:00:00:000
    LOG_I("System Uptime: " ANSI_BRIGHT_CYAN "%02lu:%02lu:%02lu:%03lu" ANSI_RESET, h, m, s, ms);
}

void cmd_print_banner(const char* a){ (void)a; term_print_banner();}

/**
 * @brief Shell Command Registry.
 * 
 * Maps user-entered strings to the corresponding C-functions.
 * Syntax: { "command_name", function_pointer, "help_text" }
 */
command_t command_config[] = {
    {"help",      cmd_help_auto,      "List all available commands", "[command]"},
	{"clear",  cmd_clear_logs,     "Clear the log area and refresh UI", NULL},
    {"uptime", cmd_uptime,         "Show system uptime in seconds",     NULL},
    {"status",    cmd_mpu_status,     "Show current hardware status", NULL},
    {"calib",     cmd_mpu_calibrate,  "Start sensor calibration", "number of samples eg. 100"},
    {"sleep", cmd_sleep,      "Enable or Disable sleep mode", "temp/device=(on/off)"},
    {"banner",     cmd_print_banner,          "Print banner", NULL},
    {"reset",     cmd_reset,          "Perform a system reboot", NULL}
};

/** @brief Total number of registered shell commands. */
const int command_count = sizeof(command_config) / sizeof(command_t);
