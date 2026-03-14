/**
 * @file terminal_cfg.c
 * @brief Project specific configuration for the terminal.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ansi-esc.h"
#include "mpu60x0.h"
#include "terminal.h"

/** @brief External reference to the automatic help generator in terminal.c. */
extern void cmd_help_auto(const char* args);

/* --- Watch Dictionary: All variables that CAN be added to the sidebar --- */
static watchable_t watch_dictionary[] = {
	{"acc_x", NULL,    TYPE_FLOAT, "g"},
	{"acc_y", NULL,    TYPE_FLOAT, "g"},
	{"acc_z", NULL,    TYPE_FLOAT, "g"},
	{"gyro_x",NULL,   TYPE_FLOAT, "°/s"},
	{"gyro_y",NULL,   TYPE_FLOAT, "°/s"},
	{"gyro_z",NULL,   TYPE_FLOAT, "°/s"},
	{"temp",  NULL, TYPE_FLOAT, "C"}
};
#define WATCH_DICT_COUNT (sizeof(watch_dictionary) / sizeof(watchable_t))

/* --- Command Implementations --- */

void cmd_mpu_status(const char* a){ 
	(void)a; 
	LOG_I("MPU6050 is %sONLINE%s", ANSI_GREEN, ANSI_RESET); 
}

void cmd_mpu_calibrate(const char* a){
	uint8_t samples = 50;
	if (a != NULL) samples = (uint8_t)atoi(a);

	LOG_I("Starting calibration with %u samples...", samples);
	mpu_calibrate((MPU_ACCEL_Z | MPU_GYRO), samples);
	LOG_I("Calibration finished.");
}

void cmd_reset(const char* a){ 
	(void)a; 
	mpu_reset(MPU_RESET_ALL); 
	LOG_W("System Reset triggered!"); 
}

void cmd_sleep(const char* a){
	if (a == NULL){
		LOG_W("Usage: sleep device=on/off OR temp=on/off");
		return;
	}

	const char* tmp_val = term_get_arg(a, "temp");

	if (strcmp(a, "on") == 0){
		mpu_sleep(MPU_SLEEP_DEVICE_ON);
		LOG_I("Sleep mode: Device %sON%s", ANSI_YELLOW, ANSI_RESET);
	}else if (strcmp(a, "off") == 0){
		mpu_sleep(MPU_SLEEP_ALL_OFF);
		LOG_I("Sleep mode: Device %sOFF%s", ANSI_GREEN, ANSI_RESET);

	}else if (tmp_val){
		if (strcmp(tmp_val, "on") == 0){
			mpu_sleep(MPU_SLEEP_TEMP_ON);
			LOG_I("Sleep mode: Temp %sON%s", ANSI_YELLOW, ANSI_RESET);
		} else if (strcmp(tmp_val, "off") == 0){
			mpu_sleep(MPU_SLEEP_TEMP_OFF);
			LOG_I("Sleep mode: Temp %sOFF%s", ANSI_GREEN, ANSI_RESET);
		}
	} else {
		LOG_E("Invalid argument. Use 'device=on/off' or 'temp=on/off'");
	}
}

void cmd_clear_logs(const char* a){
	(void)a;
	term_busy = true;
	for (int i = TERM_LOG_START; i <= TERM_LOG_END; i++){
		printf("\033[%d;1H\033[K", i);
	}
	// Refresh vertical separator
	for (int i = 1; i < TERM_CMD_ROW; i++){
		printf("\033[%d;%dH|", i, TERM_SIDEBAR_COL - 2);
	}
	LOG_I("Logs cleared.");
	term_busy = false;
}

void cmd_uptime(const char* a){
	(void)a;
	uint32_t t = to_ms_since_boot(get_absolute_time());
	uint32_t ms = t % 1000;
	uint32_t s  = (t / 1000) % 60;
	uint32_t m  = (t / 60000) % 60;
	uint32_t h  = (t / 3600000);
	LOG_I("System Uptime: %s%02lu:%02lu:%02lu:%03lu%s", ANSI_BRIGHT_CYAN, h, m, s, ms, ANSI_RESET);
}

/** @brief Command: add acc_x acc_y temp ... */
void cmd_sidebar_add(const char* a){
	if (a == NULL){
		LOG_W("Usage: add <name1> <name2> ...");
		return;
	}

	// Wir brauchen eine lokale Kopie, da strtok den String verändert
	char buf[TERM_CMD_MAX_LEN];
	strncpy(buf, a, sizeof(buf) - 1);

	char *name = strtok(buf, " ");
	while (name != NULL){
		bool found = false;
		for (int i = 0; i < WATCH_DICT_COUNT; i++){
			if (strcmp(name, watch_dictionary[i].name) == 0){
				if (term_sidebar_register(watch_dictionary[i].name, 
							watch_dictionary[i].ptr, 
							watch_dictionary[i].type, 
							watch_dictionary[i].unit)){
					LOG_I("Added: %s", name);
				} else {
					LOG_E("Sidebar full!");
				}
				found = true;
				break;
			}
		}
		if (!found) LOG_E("Unknown: %s", name);

		name = strtok(NULL, " "); // Nächstes Wort holen
	}
}

/** @brief Command: rem acc_x acc_y ... */
void cmd_sidebar_remove_bulk(const char* a){
	if (a == NULL){
		LOG_W("Usage: rem <name1> <name2> ...");
		return;
	}

	char buf[TERM_CMD_MAX_LEN];
	strncpy(buf, a, sizeof(buf) - 1);

	char *name = strtok(buf, " ");
	while (name != NULL){
		if (term_sidebar_remove(name)){
			LOG_I("Removed: %s", name);
		} else {
			LOG_E("Not active: %s", name);
		}
		name = strtok(NULL, " ");
	}
}

void cmd_list_vars(const char* a){
	(void)a;
	LOG_I("%sAvailable variables for sidebar:%s", ANSI_BOLD, ANSI_RESET);
	for (int i = 0; i < WATCH_DICT_COUNT; i++){
		LOG_I(" - %s%-10s%s [%s]", ANSI_YELLOW, watch_dictionary[i].name, ANSI_RESET, watch_dictionary[i].unit);
	}
}

void cmd_print_banner(const char* a){ (void)a; term_print_banner(); }

/* --- Shell Command Registry --- */
command_t command_config[] = {
	{"help",   cmd_help_auto,      "List all commands", "[command]"},
	{"list",   cmd_list_vars,      "List watchable variables", NULL},
	{"add",    cmd_sidebar_add,    "Add variable to sidebar", "<var> <var> <var> ..."},
	{"rem",    cmd_sidebar_remove_bulk,    "Remove variable from sidebar", "<var> <var> <var> ..."},
	{"clear",  cmd_clear_logs,     "Clear the log area", NULL},
	{"uptime", cmd_uptime,         "Show system uptime", NULL},
	{"status", cmd_mpu_status,     "Show MPU status", NULL},
	{"calib",  cmd_mpu_calibrate,  "Start calibration", "[samples]"},
	{"sleep",  cmd_sleep,          "Set sleep mode", "on/off/temp=on/off"},
	{"banner", cmd_print_banner,   "Print startup banner", NULL},
	{"reset",  cmd_reset,          "Perform system reboot", NULL}
};
const int command_count = sizeof(command_config) / sizeof(command_t);

/**
 * @brief Initialize the terminal configuration.
 */
void term_cfg_init(){
	/* Du musst jedes Element des Arrays einzeln über den Index [n] ansprechen */
	watch_dictionary[0].ptr = (void*)&(g_mpu->v.accel.g.x);
	watch_dictionary[1].ptr = (void*)&(g_mpu->v.accel.g.y);
	watch_dictionary[2].ptr = (void*)&(g_mpu->v.accel.g.z);
	watch_dictionary[3].ptr = (void*)&(g_mpu->v.gyro.dps.x);
	watch_dictionary[4].ptr = (void*)&(g_mpu->v.gyro.dps.y);
	watch_dictionary[5].ptr = (void*)&(g_mpu->v.gyro.dps.z);
	watch_dictionary[6].ptr = (void*)&(g_mpu->v.temp.celsius);
	/* Optional: Register default items to show on boot */
	term_sidebar_register("Status", (void*)"ACTIVE", TYPE_STRING, "");
}
