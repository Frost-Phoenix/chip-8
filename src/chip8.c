#include "chip8.h"

#include "cli.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>


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

static void priv_update_based_on_fps(double* last_update_time, const double target_fps, void (*update)(chip8_t*), chip8_t* chip8) {
    const double target_frame_time = 1.0 / target_fps;
    double current_time = (double)clock() / CLOCKS_PER_SEC;
    double elapsed_time = current_time - *last_update_time;

    if (elapsed_time >= target_frame_time) {
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

static void priv_update(chip8_t* chip8) {
    chip8->cpu.SP = 0;
}

// static void priv_render(chip8_t* chip8) {               /* execute when display is modified */

// }


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
    chip8->rendering_mode = mode;
    memcpy(chip8->memory + FONT_START_ARD, font, FONT_SIZE);

    if (mode == DEBUG) {
        cli_init();
    }

    signal(SIGINT, priv_signal_callback_handler);

    return chip8;
}

void chip8_quit(chip8_t* chip8) {
    if (chip8->rendering_mode == CLI || chip8->rendering_mode == DEBUG) {
        cli_quit();
    }

    free(chip8);
}

void chip8_main_loop(chip8_t* chip8) {
    double last_chip8_update = 0.0;
    double last_timer_update = 0.0;
    double last_debug_update = 0.0;

    if (chip8->rendering_mode == CLI || chip8->rendering_mode == DEBUG) {
        cli_print_display(chip8->display);
    }

    while (1) {
        priv_update_based_on_fps(&last_chip8_update, CHIP8_UPDATE_RATE, priv_update, chip8);
        priv_update_based_on_fps(&last_timer_update, TIMER_UPDATE_RATE, priv_update_timers, chip8);

        if (chip8->rendering_mode == DEBUG) {
            priv_update_based_on_fps(&last_debug_update, DEBUG_UPDATE_RATE, cli_print_debug_info, chip8);
        }
    }
}
