#if !defined(CHIP8_H)
#define CHIP8_H

#include <stdint.h>

#include "common.h"


#define CHIP8_UPDATE_RATE   700
#define TIMER_UPDATE_RATE    60
#define DEBUG_UPDATE_RATE    60

#define MEMORY_SIZE      4096
#define ROM_START_ADR   0x200
#define FONT_START_ARD   0x50
#define FONT_SIZE      16 * 5               /* 16 * 5 byte characters */

#define NB_REGISTER 16
#define STACK_SIZE  16


typedef enum {
    CLI,
    GUI,
    DEBUG,
} rendering_mode_t;

typedef struct cpu {
    uint8_t V[NB_REGISTER];                /* general purpose registers */
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

    rendering_mode_t rendering_mode;
} chip8_t;


chip8_t* chip8_init(const char* rom_path, rendering_mode_t mode);
void chip8_quit(chip8_t* chip8);

void chip8_main_loop(chip8_t* chip8);


#endif /* CHIP8_H */
