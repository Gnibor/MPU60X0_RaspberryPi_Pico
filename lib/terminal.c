/**
 * @file terminal.c
 * @brief Core rendering engine for the portable TUI (Terminal User Interface).
 * 
 * Handles the 3-zone layout: Scrolling logs (center), Static sidebar (right), 
 * and persistent command line (bottom). Supports command parsing and history.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "terminal.h"
#include "ansi-esc.h"
#include "rp_pico.h"

/* --- External Configuration (from terminal_cfg.c) --- */
extern sidebar_item_t sidebar_config[];
extern const int sidebar_count;
extern command_t command_config[];
extern const int command_count;

/** @brief Global lock to prevent UART/USB buffer collisions between Logs and Sidebar. */
volatile bool term_busy;

/* --- Internal State Buffers --- */
static char cmd_buffer[64] = {0};     /**< Current characters being typed. */
static int  hist_write_idx = 0;  /**< Index where the next new command will be saved. */
static int  hist_view_idx  = -1; /**< Navigation index while browsing (-1 = currently typing). */
static int  cmd_idx = 0;              /**< Current cursor position in command buffer. */
static char history[TERM_MAX_HISTORY][TERM_CMD_MAX_LEN] = {0}; /**< Stores the last successfully executed command. */

static void term_render_cmd_line();
static void term_autocomplete();

/**
 * @brief Prints a professional startup banner using CMake metadata.
 */
void term_print_banner(){
    LOG_I(ANSI_BOLD ANSI_CYAN "=== " APP_NAME " ===" ANSI_RESET);
    LOG_I(ANSI_DIM "Version: " ANSI_RESET APP_VERSION);
    LOG_I(ANSI_DIM "Author:  " ANSI_RESET APP_AUTHOR);
    LOG_I(ANSI_DIM "Build:   " ANSI_RESET __DATE__ " " __TIME__);
    LOG_I("Type " ANSI_YELLOW "help" ANSI_RESET " to list all available commands.");
    LOG_I("------------------------------------------");
}

/**
 * @brief Initializes the terminal screen, sets scroll regions and draws static UI elements.
 * 
 * Uses the Alternative Buffer to keep the main terminal history clean.
 */
void term_init() {
	/* Switch to Alt Buffer, clear screen, and home cursor */
	printf(ANSI_BUF_ALT ANSI_CLR ANSI_HOME);

	/* Define the scrolling region (rows only) for logs */
	printf("\033[%d;%dr", TERM_LOG_START, TERM_LOG_END);

	/* Draw Vertical Line to separate main area from sidebar */
	for(int i=1; i<TERM_CMD_ROW; i++) {
		printf("\033[%d;%dH|", i, TERM_SIDEBAR_COL - 2);
	}

	/* Draw Horizontal Separator above command line */
	printf("\033[%d;1H", TERM_CMD_ROW);
	for(int i=0; i<TERM_WIDTH; i++) printf("-");

	/* Initialize Command Prompt on the line below the separator */
	printf("\033[%d;1H" ANSI_BOLD "COMMAND" ANSI_RESET " > ", TERM_CMD_ROW + 1);

	/* Hide blinking cursor to avoid flickering during high-speed updates */
	printf(ANSI_HIDE_CUR);
	fflush(stdout);
	term_print_banner();
}

/**
 * @brief Updates the sidebar (right column) with live data from sidebar_config.
 * 
 * Uses a "try-lock" mechanism with term_busy to avoid clashing with log outputs.
 */
void term_update_sidebar() {
	/* Wait if another drawing operation is in progress */
	while(term_busy) { tight_loop_contents(); }
	term_busy = true;

	/* Save current cursor position (likely in log or command area) */
	printf("\033[s"); 

	/* Refresh vertical line (in case a log line was too long and overthrew it) */
	for(int i=1; i<TERM_CMD_ROW; i++) {
		printf("\033[%d;%dH|", i, TERM_SIDEBAR_COL - 2);
		/* Clear everything to the right of the value to prevent 'ghost' characters */
		printf("\033[K"); 
	}

	for(int i=0; i < sidebar_count; i++) {
		/* Jump to row (i+2) and sidebar start column */
		printf("\033[%d;%dH%-5s", i + 2, TERM_SIDEBAR_COL, sidebar_config[i].label);

		sidebar_item_t item = sidebar_config[i];
		if(item.type == TYPE_INT)    printf("%d %s  ", *(int16_t*)item.value_ptr, item.unit);
		if(item.type == TYPE_FLOAT)  printf("%9.2f %s  ", *(float*)item.value_ptr, item.unit);
		if(item.type == TYPE_STRING) printf("%s  ", (char*)item.value_ptr);

		/* Clear everything to the right of the value to prevent 'ghost' characters */
		printf("\033[K"); 
	}

	/* Restore cursor to previous position */
	printf("\033[u"); 
	fflush(stdout);
	term_busy = false;
}

/**
 * @brief Moves the cursor to the bottom of the log area to prepare for scrolling.
 * @note This is called by the LOG_* macros to ensure text enters the scroll region.
 */
void term_scroll_log() {
	/* Position cursor at the end of the defined log region */
	printf("\033[%d;1H", TERM_LOG_END);
	fflush(stdout);
}

/**
 * @brief Processes UART input, handles command tokenization and execution.
 */
void term_handle_input(key_t key) {
	if (key == KEY_NONE) return;

	if (key == KEY_ENTER) {
		if (cmd_idx > 0) {
			/* 1. Pre-tokenize to find the command name without modifying the buffer yet */
			char temp_buf[TERM_CMD_MAX_LEN];
			strncpy(temp_buf, cmd_buffer, TERM_CMD_MAX_LEN - 1);
			char *cmd_name = strtok(temp_buf, " ");

			bool found = false;
			int cmd_found_index = -1;

			/* 2. Search for the command in the registry */
			for (int i = 0; i < command_count; i++) {
				if (strcmp(cmd_name, command_config[i].name) == 0) {
					found = true;
					cmd_found_index = i;
					break;
				}
			}

			if (found) {
				/* 3. ONLY save to history if the command is valid and different from last entry */
				int last_saved = (hist_write_idx + TERM_MAX_HISTORY - 1) % TERM_MAX_HISTORY;
				if (strcmp(cmd_buffer, history[last_saved]) != 0) {
					strncpy(history[hist_write_idx], cmd_buffer, TERM_CMD_MAX_LEN - 1);
					hist_write_idx = (hist_write_idx + 1) % TERM_MAX_HISTORY;
				}
				LOG_I("Execute command: %s%s%s...", ANSI_ITALIC ANSI_BOLD, cmd_buffer, ANSI_RESET);
				/* 4. Actually tokenize the real buffer and execute */
				char *cmd_ptr = strtok(cmd_buffer, " ");
				(void)cmd_ptr;
				char *args_ptr = strtok(NULL, "");
				command_config[cmd_found_index].func(args_ptr);
			} else {
				/* Feedback for invalid command (not saved to history) */
				LOG_E("Command: \"%s\" unknown!", cmd_name);
			}

			/* 5. Cleanup and reset */
			hist_view_idx = -1;
			memset(cmd_buffer, 0, sizeof(cmd_buffer));
			cmd_idx = 0;
		}
	}else if (key == KEY_UP) { 
		/* Navigate UP into the past */
		int next_view;
		if (hist_view_idx == -1) {
			next_view = (hist_write_idx + TERM_MAX_HISTORY - 1) % TERM_MAX_HISTORY;
		} else {
			next_view = (hist_view_idx + TERM_MAX_HISTORY - 1) % TERM_MAX_HISTORY;
		}

		/* Only switch if there is actually a command stored at that index */
		if (strlen(history[next_view]) > 0) {
			hist_view_idx = next_view;
			strncpy(cmd_buffer, history[hist_view_idx], TERM_CMD_MAX_LEN - 1);
			cmd_idx = (int)strlen(cmd_buffer);
		}
	}else if (key == KEY_DOWN) {
		/* Navigate DOWN towards the present */
		if (hist_view_idx != -1) {
			hist_view_idx = (hist_view_idx + 1) % TERM_MAX_HISTORY;

			/* If we reached the current write index or an empty slot: Clear line */
			if (hist_view_idx == hist_write_idx || strlen(history[hist_view_idx]) == 0) {
				hist_view_idx = -1;
				memset(cmd_buffer, 0, sizeof(cmd_buffer));
				cmd_idx = 0;
			} else {
				strncpy(cmd_buffer, history[hist_view_idx], TERM_CMD_MAX_LEN - 1);
				cmd_idx = (int)strlen(cmd_buffer);
			}
		}
		/* --- BACKSPACE HANDLING --- */
	}else if(key == 9){
		term_autocomplete();
	}else if ((key == 127 || key == 8) && cmd_idx > 0) {
		/* 1. Step back in the buffer */
		cmd_idx--;
		/* 2. Null-terminate at the new position to "delete" the char */
		cmd_buffer[cmd_idx] = '\0';

		/* 3. Terminal Fix: Move cursor back, print space, move back again */
		/* This is handled by term_render_cmd_line below using ANSI_CLR_LINE */
	} else if (key >= 32 && key <= 126 && cmd_idx < 63) {
		/* Append standard ASCII character to buffer */
		cmd_buffer[cmd_idx++] = (char)key;
	}
	term_render_cmd_line();
}

static void term_render_cmd_line() {
	/* Save current cursor position (e.g. from sidebar or log) */
	printf("\033[s"); 

	/* Move to the command row and clear the entire line */
	/* ANSI_CLR_LINE (\033[2K) is crucial here! */
	printf("\033[%d;1H" ANSI_CLR_LINE, TERM_CMD_ROW + 1);

	/* Redraw the prompt and the current buffer */
	printf(ANSI_BOLD "COMMAND" ANSI_RESET " > %s", cmd_buffer);

	/* Restore cursor to previous position */
	printf("\033[u");
	fflush(stdout);
}

/**
 * @brief Searches for a key=value pair in a string.
 * @param args The full argument string (e.g., "device=on samples=100")
 * @param key  The key to search for (e.g., "samples")
 * @return Pointer to the value string, or NULL if not found.
 */
const char* term_get_arg(const char* args, const char* key) {
	if (args == NULL || key == NULL) return NULL;

	const char* p = strstr(args, key);
	while (p != NULL) {
		// Check if it's a real match (preceded by space or start, followed by '=')
		size_t key_len = strlen(key);
		if ((p == args || *(p - 1) == ' ') && *(p + key_len) == '=') {
			return p + key_len + 1; // Return pointer to the value part
		}
		p = strstr(p + 1, key); // Search next occurrence
	}
	return NULL;
}

/**
 * @brief Performs command completion or lists possibilities.
 * Standard behavior: 
 * - 1 Match: Complete the command + add space.
 * - >1 Match: List all possible commands in the log area.
 */
static void term_autocomplete() {
	if (cmd_idx == 0) return; // Nothing to complete

	int matches = 0;
	int last_match_idx = -1;

	/* 1. Find matches in the command registry */
	for (int i = 0; i < command_count; i++) {
		if (strncmp(cmd_buffer, command_config[i].name, cmd_idx) == 0) {
			matches++;
			last_match_idx = i;
		}
	}

	if (matches == 1) {
		/* Exact match found: Complete the string */
		strncpy(cmd_buffer, command_config[last_match_idx].name, TERM_CMD_MAX_LEN - 1);
		cmd_idx = (int)strlen(cmd_buffer);

		/* Add a trailing space for immediate argument input */
		if (cmd_idx < TERM_CMD_MAX_LEN - 1) {
			cmd_buffer[cmd_idx++] = ' ';
			cmd_buffer[cmd_idx] = '\0';
		}
	} 
	else if (matches > 1) {
		/* Multiple matches: List them in the log area for the user */
		LOG_I(ANSI_CYAN "Matches: " ANSI_RESET);
		for (int i = 0; i < command_count; i++) {
			if (strncmp(cmd_buffer, command_config[i].name, cmd_idx) == 0) {
				// We use printf here because LOG_I would add timestamps/newlines for each word
				printf(ANSI_YELLOW "%s  " ANSI_RESET, command_config[i].name);
			}
		}
		printf("\n"); // Move to next log line
	}
}

/**
 * @brief Automatically generates a help list based on the command_config array.
 * 
 * Formats commands with syntax highlighting (Yellow command, Dim help text).
 * @param args Unused parameter for command compatibility.
 */
void cmd_help_auto(const char* args) {
	if (args == NULL || strlen(args) == 0) {
		// --- Global Help ---
		LOG_I(ANSI_BOLD ANSI_CYAN "--- AVAILABLE COMMANDS ---" ANSI_RESET);
		for (int i = 0; i < command_count; i++) {
			LOG_I(ANSI_YELLOW " %-10s " ANSI_RESET ANSI_DIM "-> %s" ANSI_RESET, 
					command_config[i].name, command_config[i].help);
		}
		LOG_I(ANSI_DIM "Type 'help <command>' for details." ANSI_RESET);
	}else{
		// --- Specific Command Help ---
		for (int i = 0; i < command_count; i++) {
			if (strcmp(args, command_config[i].name) == 0) {
				LOG_I(ANSI_BOLD ANSI_YELLOW "HELP: %s" ANSI_RESET, command_config[i].name);
				LOG_I("Description: %s", command_config[i].help);
				if (command_config[i].usage) {
					LOG_I("Usage:       " ANSI_CYAN "%s %s" ANSI_RESET, 
							command_config[i].name, command_config[i].usage);
				}
				return;
			}
		}
		LOG_E("Help: Command '%s' not found.", args);
	}
}

