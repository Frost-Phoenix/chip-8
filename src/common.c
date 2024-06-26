#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>


static const struct option long_options [] = {
    {"help", no_argument, 0, 'h'},
    {"CLI", no_argument, 0, 'C'},
    {"GUI", no_argument, 0, 'G'},
    {"DEBUG", no_argument, 0, 'D'},
    {"ips", required_argument, 0, 'i'},
    {"scale", required_argument, 0, 's'},
    {"grid", no_argument, 0, 'g'},
    {0, 0, 0, 0}
};


/******************************************************
 *                 Private functions                  *
 ******************************************************/

static void priv_help() {
    printf("Usage: ./chip-8 <rom_path> [OPTIONS]\n\n");
    printf("Description:\n");
    printf("  Run the CHIP-8 emulator with the specified ROM file.\n\n");
    printf("Required Argument:\n");
    printf("  <rom_path>               Path to the ROM file.\n\n");
    printf("Options:\n");
    printf("  -C, --CLI                Run in CLI mode.\n");
    printf("  -G, --GUI                Run in GUI mode.\n");
    printf("  -D, --DEBUG              Run in debug mode.\n");
    printf("  -i, --ips <amount>       Number of Chip-8 instructions per seconds (default 900).\n");
    printf("\n  GUI only:\n");
    printf("  -s, --scale <amount>     Scale the display by the specified amount (default 10).\n");
    printf("  -g, --grid               Show grid on the display.\n\n");
    printf("Miscellaneous:\n");
    printf("  -h, --help               Display this help message and exit.\n");

    exit(EXIT_SUCCESS);
}

static int priv_to_int(char* input) {
    int res;
    char* end;

    res = strtol(input, &end, 10);

    if (*end != '\0') {
        printf("%serror:%s not a number.\n", "\033[1;31m", "\033[0m");
        exit(EXIT_FAILURE);
    }

    return res;
}

/******************************************************
 *                 Public functions                   *
 ******************************************************/

int get_key(const char key) {
    switch (key) {
        case '1': return 0x1;
        case '2': return 0x2;
        case '3': return 0x3;
        case '4': return 0xC;
        case 'q': case 'Q': return 0x4;
        case 'w': case 'W': return 0x5;
        case 'e': case 'E': return 0x6;
        case 'r': case 'R': return 0xD;
        case 'a': case 'A': return 0x7;
        case 's': case 'S': return 0x8;
        case 'd': case 'D': return 0x9;
        case 'f': case 'F': return 0xE;
        case 'z': case 'Z': return 0xA;
        case 'x': case 'X': return 0x0;
        case 'c': case 'C': return 0xB;
        case 'v': case 'V': return 0xF;
        default: return -1;
    }
}

void parse_args(int argc, char* argv [], args_t* args) {
    int opt;

    opterr = 0;

    if (argc < 2) {
        priv_help();
    } else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        priv_help();
    } else if (argv[1][0] == '-') {
        printf("%serror:%s first arg must be a rom path\n", "\033[1;31m", "\033[0m");
        printf("Try '%s --help' for more information.\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    args->scale = WIN_DEFAULT_SCALE;
    args->show_grid = FALSE;
    args->ips = 0;
    args->rom_path = argv[1];

    while ((opt = getopt_long(argc, argv, "hCGDi:s:g", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                priv_help();
                break;
            case 'G':
                args->rendering_mode = GUI;
                break;
            case 'C':
                args->rendering_mode = CLI;
                break;
            case 'D':
                args->rendering_mode = DEBUG;
                break;
            case 'i':
                args->ips = priv_to_int(optarg);
                break;
            case 's':
                args->scale = priv_to_int(optarg);
                break;
            case 'g':
                args->show_grid = TRUE;
                break;
            case ':':
                printf("option needs a value\n");
                break;
            case '?':
                printf("%serror:%s unrecognised flag.\n", "\033[1;31m", "\033[0m");
                printf("Try '%s --help' for more information.\n", argv[0]);
                exit(EXIT_FAILURE);
            default:
                break;
        }
    }
}