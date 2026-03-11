#ifndef ANSI_H
#define ANSI_H

// Helper macros for compile-time string concatenation
#define ANSI_STR_HELPER(x) #x                   // Converts macro arg to string
#define ANSI_STR(x) ANSI_STR_HELPER(x)          // Necessary double-macro wrapper
#define ANSI_ESC            "\033["             // The Escape Sequence prefix

// Terminal Control
#define ANSI_RESET          "\033[0m"           // Reset all text attributes
#define ANSI_CLR            "\033[2J"           // Clear entire screen buffer
#define ANSI_CLR_LINE       "\033[2K"           // Erase current line contents
#define ANSI_HOME           "\033[H"            // Move cursor to top-left (0,0)
// Move to column 1 and clear the entire line to overwrite it cleanly
#define ANSI_OVERWRITE      "\r\033[2K" 

// Text Decoration
#define ANSI_BOLD           "\033[1m"           // Make text thicker/brighter
#define ANSI_DIM            "\033[2m"           // Lower intensity of text
#define ANSI_ITALIC         "\033[3m"           // Slanted text (limited support)
#define ANSI_UNDERLINE      "\033[4m"           // Single line under text
#define ANSI_BLINK          "\033[5m"           // Flash text (use sparingly!)
#define ANSI_REVERSE        "\033[7m"           // Swap Foreground/Background colors
#define ANSI_HIDDEN         "\033[8m"           // Text exists but is invisible
#define ANSI_STRIKE         "\033[9m"           // Strike-through text decoration

// Cursor Manipulation (Literals only)
#define ANSI_HIDE_CUR       "\033[?25l"         // Stop cursor from blinking/showing
#define ANSI_SHOW_CUR       "\033[?25h"         // Restore cursor visibility
#define ANSI_GOTO(y, x)     ANSI_ESC ANSI_STR(y) ";" ANSI_STR(x) "H" // Move to Y,X
// Macro with argument for dynamic column positioning (Literals only)
#define ANSI_GOTO_COL(x)    ANSI_ESC ANSI_STR(x) "G" /* Move cursor to column X */
#define ANSI_CUR_UP(n)      ANSI_ESC ANSI_STR(n) "A" // Move cursor up n rows
#define ANSI_CUR_DOWN(n)    ANSI_ESC ANSI_STR(n) "B" // Move cursor down n rows
#define ANSI_CUR_RIGHT(n)   ANSI_ESC ANSI_STR(n) "C" // Move cursor right n columns
#define ANSI_CUR_LEFT(n)    ANSI_ESC ANSI_STR(n) "D" // Move cursor left n columns

// Standard Foreground Colors
#define ANSI_RED            "\033[31m"          // Classic Red text
#define ANSI_GREEN          "\033[32m"          // Classic Green text
#define ANSI_YELLOW         "\033[33m"          // Classic Yellow text
#define ANSI_BLUE           "\033[34m"          // Classic Blue text
#define ANSI_CYAN           "\033[36m"          // Classic Cyan text
#define ANSI_WHITE          "\033[37m"          // Classic White text

// Bright Foreground Colors
#define ANSI_BRIGHT_RED     "\033[91m"          // High-intensity Red
#define ANSI_BRIGHT_GREEN   "\033[92m"          // High-intensity Green
#define ANSI_BRIGHT_BLUE    "\033[94m"          // High-intensity Blue

// Background Colors
#define ANSI_BG_RED         "\033[41m"          // Red background highlight
#define ANSI_BG_GREEN       "\033[42m"          // Green background highlight
#define ANSI_BG_BLUE        "\033[44m"          // Blue background highlight

// Extended 256-Color Palette (Literals only)
#define ANSI_FG_256(n)      ANSI_ESC "38;5;" ANSI_STR(n) "m" // Foreground 0-255
#define ANSI_BG_256(n)      ANSI_ESC "48;5;" ANSI_STR(n) "m" // Background 0-255

#endif
