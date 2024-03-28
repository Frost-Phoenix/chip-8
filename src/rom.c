#include "rom.h"

#include <stdio.h>


rom_t* rom_loader(const char* path) {
    FILE* file;
    rom_t* rom;

    file = fopen(path, "rb");                                               /* open file */
    if (file == NULL) {
        printf("[ERROR] Cant open rom file: %s\n", path);
        exit(EXIT_FAILURE);
    }

    rom = malloc(sizeof(rom_t));
    if (rom == NULL) {
        printf("[ERROR] Cant allocate rom memory\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    if (fseek(file, 0, SEEK_END) != 0) {                                    /* go to end of file */
        printf("[ERROR] fseek failed\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    rom->len = ftell(file);                                                 /* len = delta between start - end */
    fseek(file, 0, SEEK_SET);                                               /* go back to start */

    rom->data = malloc(rom->len * sizeof(uint8_t));
    if (rom->data == NULL) {
        printf("[ERROR] Cant allocate rom memory\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    if (fread(rom->data, sizeof(uint8_t), rom->len, file) != rom->len) {   /* read entire file */
        printf("[ERROR] fread failed for file: %s\n", path);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);                                                           /* close file */

    return rom;
}

void rom_print_data(rom_t* rom) {
    printf("\nRom data: \n");

    for (size_t i = 0; i < rom->len; i++) {
        if (i % 16 == 0) {
            printf("\n");
        }
        printf("0x%02X", rom->data[i]);
        printf(" ");
    }
    printf("\n");
}

void rom_free(rom_t* rom) {
    free(rom->data);
    free(rom);
}