#include "chip8.h"

#include "cli.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>


const uint8_t font[FONT_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,       /* 0 */
    0x20, 0x60, 0x20, 0x20, 0x70,       /* 1 */
    0xF0, 0x10, 0xF0, 0x80, 0xF0,       /* 2 */
    0xF0, 0x10, 0xF0, 0x10, 0xF0,       /* 3 */
    0x90, 0x90, 0xF0, 0x10, 0x10,       /* 4 */
    0xF0, 0x80, 0xF0, 0x10, 0xF0,       /* 5 */
    0xF0, 0x80, 0xF0, 0x90, 0xF0,       /* 6 */
    0xF0, 0x10, 0x20, 0x40, 0x40,       /* 7 */
    0xF0, 0x90, 0xF0, 0x90, 0xF0,       /* 8 */
    0xF0, 0x90, 0xF0, 0x10, 0xF0,       /* 9 */
    0xF0, 0x90, 0xF0, 0x90, 0x90,       /* A */
    0xE0, 0x90, 0xE0, 0x90, 0xE0,       /* B */
    0xF0, 0x80, 0x80, 0x80, 0xF0,       /* C */
    0xE0, 0x90, 0x90, 0x90, 0xE0,       /* D */
    0xF0, 0x80, 0xF0, 0x80, 0xF0,       /* E */
    0xF0, 0x80, 0xF0, 0x80, 0x80        /* F */
};


/******************************************************
 *                 Private functions                  *
 ******************************************************/

static void priv_render(chip8_t* chip8);


static void priv_load_rom(chip8_t* chip8, const char* path) {
    FILE* file;
    uint8_t* buffer;
    size_t file_len;

    file = fopen(path, "rb");                                               /* open file */
    if (file == NULL) {
        printf("[ERROR] Cant open rom file: %s\n", path);
        exit(EXIT_FAILURE);
    }

    if (fseek(file, 0, SEEK_END) != 0) {                                    /* go to end of file */
        printf("[ERROR] fseek failed\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    file_len = ftell(file);                                                 /* len = delta between start - end */
    if (file_len > MEMORY_SIZE - ROM_START_ADR) {
        printf("[ERROR] rom to large\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_SET);                                               /* go back to start */

    buffer = malloc(file_len * sizeof(uint8_t));
    if (buffer == NULL) {
        printf("[ERROR] Cant allocate rom memory\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    if (fread(buffer, sizeof(uint8_t), file_len, file) != file_len) {   /* read entire file */
        printf("[ERROR] fread failed for file: %s\n", path);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    memcpy(chip8->memory + ROM_START_ADR, buffer, file_len);

    fclose(file);                                                           /* close file */
    free(buffer);
}

static void priv_signal_callback_handler() {
    cli_quit();
    exit(EXIT_SUCCESS);
}

static void priv_update_based_on_fps(struct timespec* last_update_time, const double target_fps, void(*update)(chip8_t*), chip8_t* chip8) {
    struct timespec current_time;
    double elapsed_time;

    clock_gettime(CLOCK_MONOTONIC, &current_time);
    elapsed_time = (double)(current_time.tv_sec - last_update_time->tv_sec) * 1.0e9 + (double)(current_time.tv_nsec - last_update_time->tv_nsec);
    elapsed_time /= 1.0e9;

    if (elapsed_time >= 1.0 / target_fps) {
        update(chip8);
        *last_update_time = current_time;
    }
}

static void priv_update_timers(chip8_t* chip8) {
    if (chip8->cpu.DT > 0) {
        chip8->cpu.DT--;
    }
    if (chip8->cpu.ST > 0) {
        chip8->cpu.ST--;
    }
}

static void priv_60Hz_update(chip8_t* chip8) {
    priv_update_timers(chip8);

    if (chip8->rendering_mode == DEBUG) {
        cli_print_debug_info(chip8);
    } else if (chip8->rendering_mode == GUI) {
        chip8->keys_last_state = chip8->keys_current_state;
        gui_poll_events(chip8->gui, &chip8->keys_current_state);
        if (chip8->gui->running == FALSE) {
            chip8->running = FALSE;
        }
        cli_print_debug_info(chip8);

    }
}

static void priv_clear_display(uint8_t* display) {
    memset(display, 0, WIN_WIDTH * WIN_HEIGHT);
}

static void priv_8XYn(chip8_t* chip8, uint8_t X, uint8_t Y, uint8_t n) {
    uint8_t flag;

    switch (n) {
        case 0x0:                                                               /* LD Vx, Vy */
            chip8->cpu.V[X] = chip8->cpu.V[Y];
            break;
        case 0x1:                                                               /* OR Vx, Vy */
            chip8->cpu.V[X] |= chip8->cpu.V[Y];
            break;
        case 0x2:                                                               /* AND Vx, Vy */
            chip8->cpu.V[X] &= chip8->cpu.V[Y];
            break;
        case 0x3:                                                               /* XOR Vx, Vy */
            chip8->cpu.V[X] ^= chip8->cpu.V[Y];
            break;
        case 0x4: {                                                               /* ADD Vx, Vy */
            uint16_t res = chip8->cpu.V[X] + chip8->cpu.V[Y];
            chip8->cpu.V[X] = res & 0xFF;
            chip8->cpu.V[0xF] = res > 0xFF;
            break;
        }
        case 0x5:                                                               /* SUB Vx, Vy */
            flag = chip8->cpu.V[X] >= chip8->cpu.V[Y];
            chip8->cpu.V[X] -= chip8->cpu.V[Y];
            chip8->cpu.V[0xF] = flag;
            break;
        case 0x6:                                                               /* SHR Vx {, Vy} */
            flag = chip8->cpu.V[X] & 0x01;
            chip8->cpu.V[X] >>= 1;
            chip8->cpu.V[0xF] = flag;
            break;
        case 0x7:                                                               /* SUBN Vx, Vy */
            flag = chip8->cpu.V[Y] >= chip8->cpu.V[X];
            chip8->cpu.V[X] = chip8->cpu.V[Y] - chip8->cpu.V[X];
            chip8->cpu.V[0xF] = flag;
            break;
        case 0xE:                                                               /* SHL Vx {, Vy} */
            flag = (chip8->cpu.V[X] >> 3) & 0x01;
            chip8->cpu.V[X] <<= 1;
            chip8->cpu.V[0xF] = flag;
            break;
        default:
            break;
    }
}

static void priv_Exnn(chip8_t* chip8, uint8_t X, uint8_t nn) {
    switch (nn) {
        case 0x9E:
            if (BIT_CHECK(chip8->keys_current_state, chip8->cpu.V[X] & 0xF)) {
                chip8->cpu.PC += 2;
            }
            break;
        case 0xA1:
            if (!BIT_CHECK(chip8->keys_current_state, chip8->cpu.V[X] & 0xF)) {
                chip8->cpu.PC += 2;
            }
            break;
        default:
            break;
    }
}

static void priv_FXnn(chip8_t* chip8, uint8_t X, uint8_t nn) {
    switch (nn) {
        case 0x07:                                                              /* LD Vx, DT */
            chip8->cpu.V[X] = chip8->cpu.DT;
            break;
        case 0x0A:
            for (size_t i = 0; i < 0xF; i++) {
                if (!BIT_CHECK(chip8->keys_current_state, i) && BIT_CHECK(chip8->keys_last_state, i)) {
                    chip8->cpu.V[X] = i;
                    return;
                }
            }
            chip8->cpu.PC -= 2;
            break;
        case 0x15:                                                              /* LD DT, Vx */
            chip8->cpu.DT = chip8->cpu.V[X];
            break;
        case 0x18:                                                              /* LD ST, Vx */
            chip8->cpu.ST = chip8->cpu.V[X];
            break;
        case 0x1E:
            chip8->cpu.I += chip8->cpu.V[X];                                    /* ADD I, Vx */
            break;
        case 0x29:                                                              /* LD F, Vx */
            chip8->cpu.I = chip8->cpu.V[X];
            break;
        case 0x33:                                                              /* LD B, Vx */
            chip8->memory[chip8->cpu.I + 0] = chip8->cpu.V[X] / 100;
            chip8->memory[chip8->cpu.I + 1] = (chip8->cpu.V[X] / 10) % 10;
            chip8->memory[chip8->cpu.I + 2] = chip8->cpu.V[X] % 10;
            break;
        case 0x55:                                                              /* LD [I], Vx */
            for (size_t i = 0; i <= X; ++i) {
                chip8->memory[chip8->cpu.I + i] = chip8->cpu.V[i];
            }
            break;
        case 0x65:                                                              /* LD Vx, [I] */
            for (size_t i = 0; i <= X; ++i) {
                chip8->cpu.V[i] = chip8->memory[chip8->cpu.I + i];
            }
            break;
        default:
            break;
    }
}

static void priv_DXYn(chip8_t* chip8, uint8_t X, uint8_t Y, uint8_t n) {
    uint8_t x, y;

    x = chip8->cpu.V[X] % WIN_WIDTH;
    y = chip8->cpu.V[Y] % WIN_HEIGHT;
    chip8->cpu.V[0xF] = 0;

    for (size_t i = 0; i < n; ++i) {
        if (y >= WIN_HEIGHT) { break; }

        uint8_t byte = chip8->memory[chip8->cpu.I + i];
        for (size_t j = 0; j < 8; ++j) {
            if (x + j >= WIN_WIDTH) { break; }

            size_t pos = y * WIN_WIDTH + (x + j);
            uint8_t pixel = (byte >> (7 - j)) & 1;

            if (pixel == 1 && chip8->display[pos] == 1) {
                chip8->cpu.V[0xF] = 1;
            }

            chip8->display[pos] ^= pixel;
        }
        ++y;
    }

    priv_render(chip8);
}

static void priv_update_chip8(chip8_t* chip8) {
    uint16_t opcode, addr;
    uint8_t n, X, Y, kk;

    opcode = (chip8->memory[chip8->cpu.PC] << 8) | (chip8->memory[chip8->cpu.PC + 1]);
    chip8->cpu.PC += 2;

    addr = opcode & 0x0FFF;
    X = (opcode & 0x0F00) >> 8;
    Y = (opcode & 0x00F0) >> 4;
    n = opcode & 0x000F;
    kk = opcode & 0x00FF;

    switch (opcode & 0xF000) {
        case 0x0000:
            if (opcode == 0x00E0) {                                             /* CLS */
                priv_clear_display(chip8->display);
            } else if (opcode == 0x00EE) {                                      /* RET */
                chip8->cpu.SP--;
                chip8->cpu.PC = chip8->cpu.stack[chip8->cpu.SP & 0xF];
            }
            break;
        case 0x1000:                                                            /* JMP addr */
            chip8->cpu.PC = addr;
            break;
        case 0x2000:                                                            /* CALL addr */
            chip8->cpu.stack[chip8->cpu.SP & 0xF] = chip8->cpu.PC;
            chip8->cpu.SP++;
            chip8->cpu.PC = addr;
            break;
        case 0x3000:                                                            /* SE V, byte */
            if (chip8->cpu.V[X] == kk) {
                chip8->cpu.PC += 2;
            }
            break;
        case 0x4000:                                                            /* SNE V, byte */
            if (chip8->cpu.V[X] != kk) {
                chip8->cpu.PC += 2;
            }
            break;
        case 0x5000:                                                            /* SE Vx, Vy */
            if (chip8->cpu.V[X] == chip8->cpu.V[Y]) {
                chip8->cpu.PC += 2;
            }
            break;
        case 0x6000:                                                            /* LD Vx, byte */
            chip8->cpu.V[X] = kk;
            break;
        case 0x7000:                                                            /* ADD Vx, byte */
            chip8->cpu.V[X] += kk;
            break;
        case 0x8000:                                                            /* see priv_8XYn() */
            priv_8XYn(chip8, X, Y, opcode & 0xF);
            break;
        case 0x9000:                                                            /* SNE Vx, Vy */
            if (chip8->cpu.V[X] != chip8->cpu.V[Y]) {
                chip8->cpu.PC += 2;
            }
            break;
        case 0xA000:                                                            /* LD I, addr */
            chip8->cpu.I = addr;
            break;
        case 0xB000:                                                            /* JP V0, addr */
            chip8->cpu.PC = addr + chip8->cpu.V[0x0];
            break;
        case 0xC000:                                                            /* RND Vx, byte */
            uint8_t r = rand() % 0x100;
            chip8->cpu.V[X] = r & kk;
            break;
        case 0xD000:                                                            /* see priv_DXYn() */
            priv_DXYn(chip8, X, Y, n);
            break;
        case 0xE000:
            priv_Exnn(chip8, X, kk);
            break;
        case 0xF000:                                                            /* see priv_FXnn() */
            priv_FXnn(chip8, X, opcode & 0x00FF);
            break;
        default:
            break;
    }
}

static void priv_render(chip8_t* chip8) {               /* execute when display is modified */
    if (chip8->rendering_mode == CLI || chip8->rendering_mode == DEBUG) {
        cli_print_display(chip8->display);
    } else if (chip8->rendering_mode == GUI) {
        gui_set_buffer(chip8->gui, chip8->display);
        gui_render(chip8->gui);
    }
}


/******************************************************
 *                 Public functions                   *
 ******************************************************/

chip8_t* chip8_init(const char* rom_path, rendering_mode_t mode) {
    chip8_t* chip8;

    chip8 = calloc(1, sizeof(chip8_t));
    if (chip8 == NULL) {
        printf("[ERROR] Cant allocate rom memory\n");
        exit(EXIT_FAILURE);
    }

    priv_load_rom(chip8, rom_path);
    chip8->cpu.PC = 0x200;
    chip8->rendering_mode = mode;
    memcpy(chip8->memory + FONT_START_ADR, font, FONT_SIZE);

    if (mode == CLI || mode == DEBUG) {
        cli_init();
    } else if (mode == GUI) {
        chip8->gui = malloc(sizeof(gui_t));
        gui_init(
            chip8->gui,
            "Chip8", SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            WIN_WIDTH * WIN_SCALE,
            WIN_HEIGHT * WIN_SCALE,
            SDL_WINDOW_SHOWN
        );
    }

    chip8->running = TRUE;

    srand(time(NULL));
    signal(SIGINT, priv_signal_callback_handler);

    priv_render(chip8);

    return chip8;
}

void chip8_quit(chip8_t* chip8) {
    if (chip8->rendering_mode == CLI || chip8->rendering_mode == DEBUG) {
        cli_quit();
    } else if (chip8->rendering_mode == GUI) {
        gui_quit(chip8->gui);
        free(chip8->gui);
    }

    free(chip8);
}

void chip8_main_loop(chip8_t* chip8) {
    struct timespec last_60Hz_update = { 0 };

    while (chip8->running) {
        priv_update_chip8(chip8);
        priv_update_based_on_fps(&last_60Hz_update, UPDATE_RATE_60Hz, priv_60Hz_update, chip8);

        usleep(1000000 / UPDATE_RATE_chip8);
    }
}
