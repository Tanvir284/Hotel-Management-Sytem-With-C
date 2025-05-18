#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "terminal_util.h"

static int hex_to_int(char ch) {
    if (ch >= '0' && ch <= '9') return ch - '0';
    ch = toupper((unsigned char)ch);
    if (ch >= 'A' && ch <= 'F') return 10 + ch - 'A';
    return 0;
}

void clear_screen(void) {
    /* ANSI escape sequence to clear screen and move cursor to top */
    printf("\033[2J\033[H");
    fflush(stdout);
}

void set_color(const char *code) {
    if (!code || strlen(code) < 2) return;
    int bg = hex_to_int(code[0]);
    int fg = hex_to_int(code[1]);
    static const int fg_colors[16] = {
        30, 34, 32, 36, 31, 35, 33, 37,
        90, 94, 92, 96, 91, 95, 93, 97
    };
    static const int bg_colors[16] = {
        40, 44, 42, 46, 41, 45, 43, 47,
        100,104,102,106,101,105,103,107
    };
    printf("\033[%d;%dm", fg_colors[fg], bg_colors[bg]);
    fflush(stdout);
}

void reset_color(void) {
    printf("\033[0m");
    fflush(stdout);
}
