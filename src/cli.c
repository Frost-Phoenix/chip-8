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
        // make cursor invisible, clear screen
        printf("\033[?25l\033[2J");
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

static void priv_display_VX_registers(const chip8_t* chip8) {
    color_t color = CYAN;

    SET_TEXT_COLOR(color);
    MOVE_CURSOR(20, 4);
    printf("┏━━━━━┓");
    RESET_FORMATING(); PRINT_BOLD("VX");
    SET_TEXT_COLOR(color); printf("┏━━━━━┓");
    MOVE_CURSOR(21, 4); printf("┃              ┃");

    for (size_t i = 0; i < STACK_SIZE; ++i) {
        MOVE_CURSOR(22 + STACK_SIZE - (int)i - 1, 4);
        SET_TEXT_COLOR(color); printf("┃  "); RESET_FORMATING();
        printf("V%1lX ", i); PRINT_DIMED("->"); printf(" 0x%02X", chip8->cpu.VX[i]);
        SET_TEXT_COLOR(color); printf("  ┃");
    }

    MOVE_CURSOR(38, 4); printf("┃              ┃");
    MOVE_CURSOR(39, 4); printf("┗━━━━━━━━━━━━━━┛");
    RESET_FORMATING();
}

static void priv_display_stack(const chip8_t* chip8) {
    color_t color = MAGENTA;

    SET_TEXT_COLOR(color);
    MOVE_CURSOR(20, 22);
    printf("┏━━━━━┓");
    RESET_FORMATING(); PRINT_BOLD("Stack");
    SET_TEXT_COLOR(color); printf("┏━━━━━┓");
    MOVE_CURSOR(21, 22); printf("┃                 ┃");

    for (size_t i = 0; i < STACK_SIZE; ++i) {
        MOVE_CURSOR(22 + STACK_SIZE - (int)i - 1, 22);
        SET_TEXT_COLOR(color); printf("┃  "); RESET_FORMATING();
        printf("0x%01lX ", i); PRINT_DIMED("->"); printf(" 0x%04X", chip8->cpu.stack[i]);
        SET_TEXT_COLOR(color); printf("  ┃");
    }

    MOVE_CURSOR(38, 22); printf("┃                 ┃");
    MOVE_CURSOR(39, 22); printf("┗━━━━━━━━━━━━━━━━━┛");
    RESET_FORMATING();
}

static void priv_display_cpu(const chip8_t* chip8) {
    MOVE_CURSOR(20, 44); PRINT_BOLD("┏━━━━━━┓Cpu┏━━━━━━┓");
    MOVE_CURSOR(21, 44); printf("┃                 ┃");

    MOVE_CURSOR(22, 44); printf("┃   PC "); PRINT_DIMED("->"); printf(" 0x%04X  ┃", chip8->cpu.PC);
    MOVE_CURSOR(23, 44); printf("┃   SC "); PRINT_DIMED("->"); printf(" 0x%02X    ┃", chip8->cpu.SP);
    MOVE_CURSOR(24, 44); printf("┃   DT "); PRINT_DIMED("->"); printf(" 0x%02X    ┃", chip8->cpu.DT);
    MOVE_CURSOR(25, 44); printf("┃   ST "); PRINT_DIMED("->"); printf(" 0x%02X    ┃", chip8->cpu.ST);
    MOVE_CURSOR(26, 44); printf("┃   I  "); PRINT_DIMED("->"); printf(" 0x%04X  ┃", chip8->cpu.I);

    MOVE_CURSOR(27, 44); printf("┃                 ┃");
    MOVE_CURSOR(28, 44); printf("┗━━━━━━━━━━━━━━━━━┛");
}


/******************************************************
 *                 Public functions                   *
 ******************************************************/

void cli_init() {
    priv_set_buffered_input(FALSE);
}

void cli_quit() {
    priv_set_buffered_input(TRUE);
}

void cli_print_display(const uint8_t* display) {
    CLEAR();

    PRINT_BOLD("┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓Game┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n");
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

void cli_print_debug_info(const chip8_t* chip8) {
    priv_display_VX_registers(chip8);
    priv_display_stack(chip8);
    priv_display_cpu(chip8);
    printf("\n");
}
