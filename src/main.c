#include <stdio.h>

#include "rom.h"


int main() {
    puts("chip-8");

    rom_t* rom = rom_loader("rom/test/2-ibm-logo.ch8");
    rom_print_data(rom);

    rom_free(rom);

    return EXIT_SUCCESS;
}
