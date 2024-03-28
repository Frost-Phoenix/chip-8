#include <stdio.h>
#include <stdlib.h>

#include "chip8.h"


int main() {
    puts("chip-8");

    chip8_t* chip8 = chip8_init("rom/test/2-ibm-logo.ch8");

    chip8_quit(chip8);

    return EXIT_SUCCESS;
}
