#include <stdio.h>
#include <stdlib.h>

#include "chip8.h"


int main() {
    puts("chip-8");

    // chip8_t* chip8 = chip8_init("rom/test/1-chip8-logo.ch8", DEBUG);
    // chip8_t* chip8 = chip8_init("rom/test/2-ibm-logo.ch8", DEBUG);
    // chip8_t* chip8 = chip8_init("rom/test/3-corax+.ch8", DEBUG);
    // chip8_t* chip8 = chip8_init("rom/test/4-flags.ch8", DEBUG);
    // chip8_t* chip8 = chip8_init("rom/test/1-chip8-logo.ch8", GUI);
    // chip8_t* chip8 = chip8_init("rom/test/2-ibm-logo.ch8", GUI);
    // chip8_t* chip8 = chip8_init("rom/test/3-corax+.ch8", GUI);
    // chip8_t* chip8 = chip8_init("rom/test/4-flags.ch8", GUI);
    // chip8_t* chip8 = chip8_init("rom/test/5-quirks.ch8", GUI);
    chip8_t* chip8 = chip8_init("rom/test/6-keypad.ch8", GUI);
    // chip8_t* chip8 = chip8_init("rom/games/Breakout [Carmelo Cortez, 1979].ch8", GUI);
    // chip8_t* chip8 = chip8_init("rom/games/Tetris [Fran Dachille, 1991].ch8", GUI);
    chip8_main_loop(chip8);
    chip8_quit(chip8);

    return EXIT_SUCCESS;
}
