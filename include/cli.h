#if !defined(CLI_H)
#define CLI_H

#include <stdio.h>
#include <stdint.h>


#define CLEAR()             printf("\033[H\033[J")
#define SET_TEXT_COLOR(X)   printf("\033[1;3%dm", X)
#define SET_BG_COLOR(X)     printf("\033[4%dm", X)
#define MOVE_CURSOR(R, C)   printf("\033[%d;%dH", R, C)
#define RESET_FORMATING()   printf("\033[0m")

typedef enum {
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE
} Color;


void cli_init();
void cli_quit();

void cli_print_display(const uint8_t* display);


#endif /* CLI_H */
