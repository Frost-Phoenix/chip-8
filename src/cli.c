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
    color_t color = GREEN;

    SET_TEXT_COLOR(color);
    MOVE_CURSOR(20, 6);
    printf("┏━━━━━┓");
    RESET_FORMATING(); PRINT_BOLD("VX");
    SET_TEXT_COLOR(color); printf("┏━━━━━┓");
    MOVE_CURSOR(21, 6); printf("┃              ┃");

    for (size_t i = 0; i < STACK_SIZE; ++i) {
        MOVE_CURSOR(22 + STACK_SIZE - (int)i - 1, 6);
        SET_TEXT_COLOR(color); printf("┃  "); RESET_FORMATING();
        printf("V%1lX ", i); PRINT_DIMED("->"); printf(" 0x%02X", chip8->cpu.VX[i]);
        SET_TEXT_COLOR(color); printf("  ┃");
    }

    MOVE_CURSOR(38, 6); printf("┃              ┃");
    MOVE_CURSOR(39, 6); printf("┗━━━━━━━━━━━━━━┛");
    RESET_FORMATING();
}

static void priv_display_stack(const chip8_t* chip8) {
    color_t color = RED;

    SET_TEXT_COLOR(color);
    MOVE_CURSOR(20, 24);
    printf("┏━━━━━┓");
    RESET_FORMATING(); PRINT_BOLD("Stack");
    SET_TEXT_COLOR(color); printf("┏━━━━━┓");
    MOVE_CURSOR(21, 24); printf("┃                 ┃");

    for (size_t i = 0; i < STACK_SIZE; ++i) {
        MOVE_CURSOR(22 + STACK_SIZE - (int)i - 1, 24);
        SET_TEXT_COLOR(color); printf("┃  "); RESET_FORMATING();
        printf("0x%01lX ", i); PRINT_DIMED("->"); printf(" 0x%04X", chip8->cpu.stack[i]);
        SET_TEXT_COLOR(color); printf("  ┃");
    }

    MOVE_CURSOR(38, 24); printf("┃                 ┃");
    MOVE_CURSOR(39, 24); printf("┗━━━━━━━━━━━━━━━━━┛");
    RESET_FORMATING();
}

static void priv_display_cpu(const chip8_t* chip8) {
    color_t color = BLUE;

    SET_TEXT_COLOR(color);
    MOVE_CURSOR(20, 45);
    printf("┏━━━━━━┓");
    RESET_FORMATING(); PRINT_BOLD("Cpu");
    SET_TEXT_COLOR(color); printf("┏━━━━━━┓");
    MOVE_CURSOR(21, 45); printf("┃                 ┃");

    MOVE_CURSOR(22, 45); SET_TEXT_COLOR(color); printf("┃   "); RESET_FORMATING();
    printf("PC "); PRINT_DIMED("->"); printf(" 0x%04X", chip8->cpu.PC); SET_TEXT_COLOR(color); printf("  ┃");
    MOVE_CURSOR(23, 45); SET_TEXT_COLOR(color); printf("┃   "); RESET_FORMATING();
    printf("SC "); PRINT_DIMED("->"); printf(" 0x%02X", chip8->cpu.SP); SET_TEXT_COLOR(color); printf("    ┃");
    MOVE_CURSOR(24, 45); SET_TEXT_COLOR(color); printf("┃   "); RESET_FORMATING();
    printf("DT "); PRINT_DIMED("->"); printf(" 0x%02X", chip8->cpu.DT); SET_TEXT_COLOR(color); printf("    ┃");
    MOVE_CURSOR(25, 45); SET_TEXT_COLOR(color); printf("┃   "); RESET_FORMATING();
    printf("ST "); PRINT_DIMED("->"); printf(" 0x%02X", chip8->cpu.ST); SET_TEXT_COLOR(color); printf("    ┃");
    MOVE_CURSOR(26, 45); SET_TEXT_COLOR(color); printf("┃   "); RESET_FORMATING();
    printf("I  "); PRINT_DIMED("->"); printf(" 0x%04X", chip8->cpu.I); SET_TEXT_COLOR(color); printf("  ┃");

    MOVE_CURSOR(27, 45); printf("┃                 ┃");
    MOVE_CURSOR(28, 45); printf("┗━━━━━━━━━━━━━━━━━┛");
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

void cli_print_memory(const chip8_t* chip8) {
    for (size_t i = 0; i < MEMORY_SIZE; i++) {
        if (i % 32 == 0) {
            printf("\n");
        }
        uint8_t val = chip8->memory[MEMORY_SIZE - i - 1];
        if (val == 0x00) {
            printf("\033[2m0x%02X\033[0m ", val);
        } else {
            printf("0x%02X ", val);
        }
    }
    printf("\n");
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

void cli_print_debug_info(chip8_t* chip8) {
    priv_display_VX_registers(chip8);
    priv_display_stack(chip8);
    priv_display_cpu(chip8);
    printf("\n");
}
