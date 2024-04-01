#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "common.h"
#include "chip8.h"


int main(int argc, char* argv []) {
    chip8_t* chip8;
    args_t args = { 0 };

    parse_args(argc, argv, &args);

    chip8 = chip8_init(args.rom_path, args.rendering_mode, args.scale, args.show_grid);
    chip8_main_loop(chip8);
    chip8_quit(chip8);

    exit(EXIT_SUCCESS);
}
