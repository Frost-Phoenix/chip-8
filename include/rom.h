#ifndef ROM_H
#define ROM_H

#include <stdint.h>
#include <stdlib.h>


typedef struct rom {
    uint8_t* data;
    size_t len;
} rom_t;



rom_t* rom_loader(const char* path);
void rom_print_data(rom_t* rom);

void rom_free(rom_t* rom);


#endif /* ROM_H */