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

static void priv_delayed_update(struct timespec* last_update_time, const double target_fps, void(*update)(chip8_t*), chip8_t* chip8) {
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
    cpu_t* cpu = &chip8->cpu;

    if (cpu->DT > 0) {
        cpu->DT--;
    }
    if (cpu->ST > 0) {
        cpu->ST--;
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
    }
}

static void priv_clear_display(uint8_t* display) {
    memset(display, 0, WIN_WIDTH * WIN_HEIGHT);
}

static void priv_8XYn(chip8_t* chip8, uint8_t X, uint8_t Y, uint8_t n) {
    cpu_t* cpu = &chip8->cpu;
    uint8_t flag;

    switch (n) {
        case 0x0:                                                               /* LD Vx, Vy */
            cpu->V[X] = cpu->V[Y];
            break;
        case 0x1:                                                               /* OR Vx, Vy */
            cpu->V[X] |= cpu->V[Y];
            cpu->V[0xF] = 0;
            break;
        case 0x2:                                                               /* AND Vx, Vy */
            cpu->V[X] &= cpu->V[Y];
            cpu->V[0xF] = 0;
            break;
        case 0x3:                                                               /* XOR Vx, Vy */
            cpu->V[X] ^= cpu->V[Y];
            cpu->V[0xF] = 0;
            break;
        case 0x4: {                                                               /* ADD Vx, Vy */
            uint16_t res = cpu->V[X] + cpu->V[Y];
            cpu->V[X] = res & 0xFF;
            cpu->V[0xF] = res > 0xFF;
            break;
        }
        case 0x5:                                                               /* SUB Vx, Vy */
            flag = cpu->V[X] >= cpu->V[Y];
            cpu->V[X] -= cpu->V[Y];
            cpu->V[0xF] = flag;
            break;
        case 0x6:                                                               /* SHR Vx {, Vy} */
            cpu->V[X] = cpu->V[Y];
            flag = cpu->V[X] & 0x01;
            cpu->V[X] >>= 1;
            cpu->V[0xF] = flag;
            break;
        case 0x7:                                                               /* SUBN Vx, Vy */
            flag = cpu->V[Y] >= cpu->V[X];
            cpu->V[X] = cpu->V[Y] - cpu->V[X];
            cpu->V[0xF] = flag;
            break;
        case 0xE:                                                               /* SHL Vx {, Vy} */
            cpu->V[X] = cpu->V[Y];
            flag = (cpu->V[X] >> 7) & 0x01;
            cpu->V[X] <<= 1;
            cpu->V[0xF] = flag;
            break;
        default:
            break;
    }
}

static void priv_Exnn(chip8_t* chip8, uint8_t X, uint8_t nn) {
    cpu_t* cpu = &chip8->cpu;

    switch (nn) {
        case 0x9E:                                                              /* SKP Vx */
            if (BIT_CHECK(chip8->keys_current_state, cpu->V[X] & 0xF)) {
                cpu->PC += 2;
            }
            break;
        case 0xA1:                                                              /* SKNP Vx */
            if (!BIT_CHECK(chip8->keys_current_state, cpu->V[X] & 0xF)) {
                cpu->PC += 2;
            }
            break;
        default:
            break;
    }
}

static void priv_FXnn(chip8_t* chip8, uint8_t X, uint8_t nn) {
    cpu_t* cpu = &chip8->cpu;

    switch (nn) {
        case 0x07:                                                              /* LD Vx, DT */
            cpu->V[X] = cpu->DT;
            break;
        case 0x0A:                                                              /* LD Vx, K */
            for (size_t i = 0; i < 0xF; i++) {
                if (!BIT_CHECK(chip8->keys_current_state, i) && BIT_CHECK(chip8->keys_last_state, i)) {
                    cpu->V[X] = i;
                    return;
                }
            }
            cpu->PC -= 2;
            break;
        case 0x15:                                                              /* LD DT, Vx */
            cpu->DT = cpu->V[X];
            break;
        case 0x18:                                                              /* LD ST, Vx */
            cpu->ST = cpu->V[X];
            break;
        case 0x1E:                                                              /* ADD I, Vx */
            cpu->I += cpu->V[X];
            break;
        case 0x29:                                                              /* LD F, Vx */
            cpu->I = FONT_START_ADR + (cpu->V[X] & 0xF) * 5;
            break;
        case 0x33:                                                              /* LD B, Vx */
            chip8->memory[cpu->I + 0] = cpu->V[X] / 100;
            chip8->memory[cpu->I + 1] = (cpu->V[X] / 10) % 10;
            chip8->memory[cpu->I + 2] = cpu->V[X] % 10;
            break;
        case 0x55:                                                              /* LD [I], Vx */
            for (size_t i = 0; i <= X; ++i) {
                chip8->memory[cpu->I++] = cpu->V[i];
            }
            break;
        case 0x65:                                                              /* LD Vx, [I] */
            for (size_t i = 0; i <= X; ++i) {
                cpu->V[i] = chip8->memory[cpu->I++];
            }
            break;
        default:
            break;
    }
}

static void priv_DXYn(chip8_t* chip8, uint8_t X, uint8_t Y, uint8_t n) {
    cpu_t* cpu = &chip8->cpu;
    uint8_t x, y;

    x = cpu->V[X] % WIN_WIDTH;
    y = cpu->V[Y] % WIN_HEIGHT;
    cpu->V[0xF] = 0;

    for (size_t i = 0; i < n; ++i) {
        if (y >= WIN_HEIGHT) { break; }

        uint8_t byte = chip8->memory[cpu->I + i];
        for (size_t j = 0; j < 8; ++j) {
            if (x + j >= WIN_WIDTH) { break; }

            size_t pos = y * WIN_WIDTH + (x + j);
            uint8_t pixel = (byte >> (7 - j)) & 1;

            if (pixel == 1 && chip8->display[pos] == 1) {
                cpu->V[0xF] = 1;
            }

            chip8->display[pos] ^= pixel;
        }
        ++y;
    }

    priv_render(chip8);
}

static void priv_update_chip8(chip8_t* chip8) {
    cpu_t* cpu = &chip8->cpu;
    uint16_t opcode, addr;
    uint8_t n, X, Y, kk;

    opcode = (chip8->memory[cpu->PC] << 8) | (chip8->memory[cpu->PC + 1]);
    cpu->PC += 2;

    addr = opcode & 0x0FFF;
    X = (opcode & 0x0F00) >> 8;
    Y = (opcode & 0x00F0) >> 4;
    n = opcode & 0x000F;
    kk = opcode & 0x00FF;

    switch ((opcode & 0xF000) >> 12) {
        case 0x0:
            if (opcode == 0x00E0) {                                             /* CLS */
                priv_clear_display(chip8->display);
            } else if (opcode == 0x00EE) {                                      /* RET */
                cpu->SP--;
                cpu->PC = cpu->stack[cpu->SP & 0xF];
            }
            break;
        case 0x1:                                                            /* JMP addr */
            cpu->PC = addr;
            break;
        case 0x2:                                                            /* CALL addr */
            cpu->stack[cpu->SP & 0xF] = cpu->PC;
            cpu->SP++;
            cpu->PC = addr;
            break;
        case 0x3:                                                            /* SE V, byte */
            if (cpu->V[X] == kk) {
                cpu->PC += 2;
            }
            break;
        case 0x4:                                                            /* SNE V, byte */
            if (cpu->V[X] != kk) {
                cpu->PC += 2;
            }
            break;
        case 0x5:                                                            /* SE Vx, Vy */
            if (n == 0x0 && cpu->V[X] == cpu->V[Y]) {
                cpu->PC += 2;
            }
            break;
        case 0x6:                                                            /* LD Vx, byte */
            cpu->V[X] = kk;
            break;
        case 0x7:                                                            /* ADD Vx, byte */
            cpu->V[X] += kk;
            break;
        case 0x8:                                                            /* see priv_8XYn() */
            priv_8XYn(chip8, X, Y, opcode & 0xF);
            break;
        case 0x9:                                                            /* SNE Vx, Vy */
            if (n == 0x0 && cpu->V[X] != cpu->V[Y]) {
                cpu->PC += 2;
            }
            break;
        case 0xA:                                                            /* LD I, addr */
            cpu->I = addr;
            break;
        case 0xB:                                                            /* JP V0, addr */
            cpu->PC = addr + cpu->V[0x0];
            break;
        case 0xC:                                                            /* RND Vx, byte */
            uint8_t r = rand() % 0x100;
            cpu->V[X] = r & kk;
            break;
        case 0xD:                                                            /* see priv_DXYn() */
            priv_DXYn(chip8, X, Y, n);
            break;
        case 0xE:                                                            /* see priv_EXnn() */
            priv_Exnn(chip8, X, kk);
            break;
        case 0xF:                                                            /* see priv_FXnn() */
            priv_FXnn(chip8, X, opcode & 0x00FF);
            break;
        default:
            break;
    }
}

static void priv_render(chip8_t* chip8) {                                       /* execute when display is modified */
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

chip8_t* chip8_init(const char* rom_path, rendering_mode_t mode, int scale) {
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
            WIN_WIDTH * scale,
            WIN_HEIGHT * scale,
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
        priv_delayed_update(&last_60Hz_update, UPDATE_RATE_60HZ, priv_60Hz_update, chip8);

        usleep(1000000 / UPDATE_RATE_CHIP8);
    }
}
