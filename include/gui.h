#if !defined(GUI_H)
#define GUI_H

#include <stdint.h>
#include <raylib.h>

#include "common.h"


typedef struct gui {
    int running;
    int scale;
    int show_grid;

    uint8_t buffer[CHIP8_DISPLAY_WIDTH * CHIP8_DISPLAY_HEIGHT * 3];

    Texture2D texture;
    Rectangle source, dest;
} gui_t;


void gui_init(gui_t* gui, const char* title, int scale, int show_grid);
void gui_quit();

void gui_poll_events(gui_t* gui, uint16_t* keys_state);
void gui_set_buffer(gui_t* gui, uint8_t* buffer);
void gui_render(gui_t* gui);


#endif // GUI_H
