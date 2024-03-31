#include "cli.h"

#include <unistd.h>
#include <termios.h>
#include <sys/select.h>

#include "common.h"


/******************************************************
 *                 Private functions                  *
 ******************************************************/

static void priv_set_buffered_input(int enable) {
    static int enabled = 1;
    static struct termios old;
    struct termios new;

    if (enable && !enabled) {
        tcsetattr(STDIN_FILENO, TCSANOW, &old);                             /* restore the former settings */
        printf("\033[?25h\033[m");                                          /* make cursor visible, reset all modes */
        enabled = 1;
    } else if (!enable && enabled) {
        tcgetattr(STDIN_FILENO, &new);                                      /* get the terminal settings for standard input */
        printf("\033[?25l\033[2J");                                         /* make cursor invisible, clear screen */
        old = new;                                                          /* keep old setting to restore them at the end */
        new.c_lflag &= (~ICANON & ~ECHO);                                   /* disable canonical mode (buffered i/o) and local echo */
        tcsetattr(STDIN_FILENO, TCSANOW, &new);                             /* set the new settings immediately */
        enabled = 0;
    }
}

static uint16_t priv_get_key_pressed() {
    int key = 0;
    fd_set read_fds;
    struct timeval timeout;

    FD_ZERO(&read_fds);
    FD_SET(STDIN_FILENO, &read_fds);

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    if (select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &timeout) > 0) {
        key = getchar();
        fflush(stdin);
    }

    return key;
}

static void priv_display_VX_registers(const chip8_t* chip8) {
    color_t color = GREEN;

    SET_TEXT_COLOR(color);
    MOVE_CURSOR(20, 5);
    printf("┏━━━━━┓");
    RESET_FORMATING(); PRINT_BOLD("VX");
    SET_TEXT_COLOR(color); printf("┏━━━━━┓");
    MOVE_CURSOR(21, 5); printf("┃              ┃");

    for (size_t i = 0; i < STACK_SIZE; ++i) {
        MOVE_CURSOR(22 + STACK_SIZE - (int)i - 1, 5);
        SET_TEXT_COLOR(color); printf("┃  "); RESET_FORMATING();
        printf("V%1lX ", i); PRINT_DIMED("->"); printf(" 0x%02X", chip8->cpu.V[i]);
        SET_TEXT_COLOR(color); printf("  ┃");
    }

    MOVE_CURSOR(38, 5); printf("┃              ┃");
    MOVE_CURSOR(39, 5); printf("┗━━━━━━━━━━━━━━┛");
    RESET_FORMATING();
}

static void priv_display_stack(const chip8_t* chip8) {
    color_t color = RED;

    SET_TEXT_COLOR(color);
    MOVE_CURSOR(20, 23);
    printf("┏━━━━━┓");
    RESET_FORMATING(); PRINT_BOLD("Stack");
    SET_TEXT_COLOR(color); printf("┏━━━━━┓");
    MOVE_CURSOR(21, 23); printf("┃                 ┃");

    for (size_t i = 0; i < STACK_SIZE; ++i) {
        MOVE_CURSOR(22 + STACK_SIZE - (int)i - 1, 23);
        SET_TEXT_COLOR(color); printf("┃  "); RESET_FORMATING();
        printf("0x%01lX ", i); PRINT_DIMED("->"); printf(" 0x%04X", chip8->cpu.stack[i]);
        SET_TEXT_COLOR(color); printf("  ┃");
    }

    MOVE_CURSOR(38, 23); printf("┃                 ┃");
    MOVE_CURSOR(39, 23); printf("┗━━━━━━━━━━━━━━━━━┛");
    RESET_FORMATING();
}

static void priv_display_cpu(const chip8_t* chip8) {
    color_t color = BLUE;

    SET_TEXT_COLOR(color);
    MOVE_CURSOR(20, 44);
    printf("┏━━━━━━━┓");
    RESET_FORMATING(); PRINT_BOLD("Cpu");
    SET_TEXT_COLOR(color); printf("┏━━━━━━━┓");
    MOVE_CURSOR(21, 44); printf("┃                   ┃");

    MOVE_CURSOR(22, 44); SET_TEXT_COLOR(color); printf("┃    "); RESET_FORMATING();
    printf("PC "); PRINT_DIMED("->"); printf(" 0x%04X", chip8->cpu.PC); SET_TEXT_COLOR(color); printf("   ┃");
    MOVE_CURSOR(23, 44); SET_TEXT_COLOR(color); printf("┃    "); RESET_FORMATING();
    printf("SP "); PRINT_DIMED("->"); printf(" 0x%02X", chip8->cpu.SP); SET_TEXT_COLOR(color); printf("     ┃");
    MOVE_CURSOR(24, 44); SET_TEXT_COLOR(color); printf("┃    "); RESET_FORMATING();
    printf("DT "); PRINT_DIMED("->"); printf(" 0x%02X", chip8->cpu.DT); SET_TEXT_COLOR(color); printf("     ┃");
    MOVE_CURSOR(25, 44); SET_TEXT_COLOR(color); printf("┃    "); RESET_FORMATING();
    printf("ST "); PRINT_DIMED("->"); printf(" 0x%02X", chip8->cpu.ST); SET_TEXT_COLOR(color); printf("     ┃");
    MOVE_CURSOR(26, 44); SET_TEXT_COLOR(color); printf("┃    "); RESET_FORMATING();
    printf("I  "); PRINT_DIMED("->"); printf(" 0x%04X", chip8->cpu.I); SET_TEXT_COLOR(color); printf("   ┃");
    MOVE_CURSOR(27, 44); printf("┃                   ┃");
    MOVE_CURSOR(28, 44); SET_TEXT_COLOR(color); printf("┃"); RESET_FORMATING();
    printf("    Keys state:    "); SET_TEXT_COLOR(color); printf("┃");
    MOVE_CURSOR(29, 44); SET_TEXT_COLOR(color); printf("┃ "); RESET_FORMATING();
    printf(BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(chip8->keys_current_state >> 8), BYTE_TO_BINARY(chip8->keys_current_state));
    SET_TEXT_COLOR(color); printf(" ┃");

    MOVE_CURSOR(30, 44); printf("┃                   ┃");
    MOVE_CURSOR(31, 44); printf("┗━━━━━━━━━━━━━━━━━━━┛");
    RESET_FORMATING();
}


/******************************************************
 *                 Public functions                   *
 ******************************************************/

void cli_init() {
    CLEAR();
    priv_set_buffered_input(FALSE);
}

void cli_quit() {
    priv_set_buffered_input(TRUE);
}

uint16_t cli_get_keys() {
    char key = priv_get_key_pressed();

    return key != 0 ? 1 << get_key(key) : 0;
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
    MOVE_CURSOR(0, 0);
    PRINT_BOLD("╔═════════════════════════════╗Chip-8╔═════════════════════════════╗\n");
    for (size_t r = 0; r < WIN_HEIGHT; r += 2) {
        printf("║ ");
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
        printf(" ║\n");
    }
    printf("╚══════════════════════════════════════════════════════════════════╝\n");

    RESET_FORMATING();
}

void cli_print_debug_info(chip8_t* chip8) {
    priv_display_VX_registers(chip8);
    priv_display_stack(chip8);
    priv_display_cpu(chip8);
    printf("\n");
}
