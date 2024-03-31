#if !defined(COMMON_H)
#define COMMON_H


#define TRUE  1
#define FALSE 0

#define WIN_DEFAULT_SCALE   10
#define WIN_WIDTH   64
#define WIN_HEIGHT  32

#define BIT_CHECK(X, N) ((X) & (1 << (N)))
#define BIT_SET(X, N)   ((X) |= (1 << (N)))
#define BIT_CLEAR(X, N) ((X) &= ~(1 << (N)))

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)   \
    ((byte) & 0x80 ? '1' : '0'), \
    ((byte) & 0x40 ? '1' : '0'), \
    ((byte) & 0x20 ? '1' : '0'), \
    ((byte) & 0x10 ? '1' : '0'), \
    ((byte) & 0x08 ? '1' : '0'), \
    ((byte) & 0x04 ? '1' : '0'), \
    ((byte) & 0x02 ? '1' : '0'), \
    ((byte) & 0x01 ? '1' : '0') 



typedef enum {
    GUI = 0,
    CLI,
    DEBUG,
} rendering_mode_t;

typedef struct args {
    char* rom_path;
    int scale;
    rendering_mode_t rendering_mode;
    int show_grid;
} args_t;


int get_key(const char key);
void parse_args(int argc, char* argv [], args_t* args);


#endif /* COMMON_H */