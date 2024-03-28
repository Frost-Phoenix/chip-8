#if !defined(CHIP8_H)
#define CHIP8_H

#include <stdint.h>

#include "common.h"


#define MEMORY_SIZE    4096
#define ROM_START_ADR 0x200

#define NB_REGISTER 16
#define STACK_SIZE  16


typedef struct cpu {
    uint8_t Vx[NB_REGISTER];                /* general purpose registers */
    uint8_t DT, ST;                         /* delay and sound timer */
    uint16_t I;                             /* register that usualy stores memoty adresses */

    uint8_t SP;                             /* stack pointer */
    uint16_t PC;                            /* program counter */
    uint16_t stack[STACK_SIZE];
} cpu_t;

typedef struct chip8 {
    cpu_t cpu;
    uint8_t memory[MEMORY_SIZE];
    uint8_t display[WIN_WIDTH * WIN_HEIGHT];
} chip8_t;



chip8_t* chip8_init(const char* rom_path);

void chip8_quit(chip8_t* chip8);


#endif /* CHIP8_H */
