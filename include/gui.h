#if !defined(GUI_H)
#define GUI_H

#include <SDL2/SDL.h>

#include "common.h"


typedef struct gui {
    SDL_Window* win;
    SDL_Renderer* ren;

    uint8_t buffer[WIN_WIDTH * WIN_HEIGHT * 3];
    SDL_Texture* texture;

    int running;
    SDL_Event event;
} gui_t;


void gui_init(gui_t* gui, const char* title, int x, int y, int w, int h, const Uint32 flags);
void gui_quit(gui_t* gui);

void gui_poll_events(gui_t* gui);
void gui_set_buffer(gui_t* gui, uint8_t* buffer);
void gui_render(gui_t* gui);


#endif // GUI_H
