#include "chip8.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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

    if (mode == DEBUG) {
        cli_init();
    }

    return chip8;
}

void chip8_quit(chip8_t* chip8) {
    if (chip8->rendering_mode == CLI || chip8->rendering_mode == DEBUG) {
        cli_quit();
    }

    free(chip8);
}