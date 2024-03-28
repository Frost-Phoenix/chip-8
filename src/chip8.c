#include "chip8.h"

#include <stdio.h>
#include <stdlib.h>


chip8_t* chip8_init(const char* rom_path) {
    chip8_t* chip8;
    rom_t* rom = rom_loader(rom_path);

    chip8 = calloc(1, sizeof(chip8_t));
    if (chip8 == NULL) {
        printf("[ERROR] Cant allocate rom memory\n");
        exit(EXIT_FAILURE);
    }

    chip8->rom = rom;

    return chip8;
}


void chip8_quit(chip8_t* chip8) {
    rom_free(chip8->rom);
    free(chip8);
}