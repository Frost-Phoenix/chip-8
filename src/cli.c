#include "cli.h"

#include <unistd.h>
#include <termios.h>

#include "common.h"


/******************************************************
 *                 Private functions                  *
 ******************************************************/

static void priv_set_buffered_input(int enable) {
    static int enabled = 1;
    static struct termios old;
    struct termios new;

    if (enable && !enabled) {
        // restore the former settings
        tcsetattr(STDIN_FILENO, TCSANOW, &old);
        // make cursor visible, reset all modes
        printf("\033[?25h\033[m");
        // set the new state
        enabled = 1;
    } else if (!enable && enabled) {
        // get the terminal settings for standard input
        tcgetattr(STDIN_FILENO, &new);
        // we want to keep the old setting to restore them at the end
        old = new;
        // disable canonical mode (buffered i/o) and local echo
        new.c_lflag &= (~ICANON & ~ECHO);
        // set the new settings immediately
        tcsetattr(STDIN_FILENO, TCSANOW, &new);
        // set the new state
        enabled = 0;
    }
}

/******************************************************
 *                 Public functions                   *
 ******************************************************/

void cli_init() {
    priv_set_buffered_input(TRUE);
}

void cli_quit() {
    priv_set_buffered_input(FALSE);
}

void cli_print_display(const uint8_t* display) {
    CLEAR();

    printf("┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\033[1mtest\033[0m┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n");
    for (size_t r = 0; r < WIN_HEIGHT; r += 2) {
        printf("┃ ");
        for (size_t c = 0; c < WIN_WIDTH; c++) {
            SET_BG_COLOR(BLACK);
            SET_TEXT_COLOR(BLACK);

            if (display[r * WIN_WIDTH + c] == 1) {
                SET_BG_COLOR(WHITE);
            }
            if (display[(r + 1) * WIN_WIDTH + c] == 1) {
                SET_TEXT_COLOR(WHITE);
            }

            printf("▄");
        }
        RESET_FORMATING();
        printf(" ┃\n");
    }
    printf("┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n");

    RESET_FORMATING();
}
