#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "chip8.h"
#include "cli.h"


int main() {
    puts("chip-8");

    chip8_t* chip8 = chip8_init("rom/test/2-ibm-logo.ch8", DEBUG);

    cli_print_display(chip8->display);
    cli_print_debug_info(chip8);

    srand(time(NULL));

    while (1) {
        chip8->cpu.VX[rand() % 16] = rand() % 0xff;
        chip8->cpu.stack[rand() % 16] = rand() % 0xffff;
        chip8->cpu.PC = rand() % 0xffff;
        chip8->cpu.I = rand() % 0xffff;
        chip8->cpu.SP = rand() % 0xff;
        chip8->cpu.DT = rand() % 0xff;
        chip8->cpu.ST = rand() % 0xff;

        cli_print_display(chip8->display);
        cli_print_debug_info(chip8);

        usleep(1000000 / 60);
    }

    getchar();

    chip8_quit(chip8);

    return EXIT_SUCCESS;
}
