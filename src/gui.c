#include "gui.h"


/******************************************************
 *                 Private functions                  *
 ******************************************************/




 /******************************************************
  *                 Public functions                   *
  ******************************************************/

void gui_init(gui_t* gui, const char* title, int x, int y, int w, int h, const Uint32 flags) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("[ERROR] Unable to init window: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    gui->win = SDL_CreateWindow(title, x, y, w, h, flags);
    if (gui->win == NULL) {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    gui->ren = SDL_CreateRenderer(
        gui->win,
        -1,
        SDL_RENDERER_ACCELERATED
    );
    if (gui->win == NULL) {
        SDL_Log("Unable to create renderer: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_RenderSetLogicalSize(gui->ren, WIN_WIDTH, WIN_HEIGHT);

    gui->texture = SDL_CreateTexture(
        gui->ren,
        SDL_PIXELFORMAT_RGB24,                                      /* RR GG BB */
        SDL_TEXTUREACCESS_STREAMING,
        WIN_WIDTH,
        WIN_HEIGHT
    );
    if (gui->texture == NULL) {
        SDL_Log("Unable to create texture: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    memset(gui->buffer, 0, sizeof(uint8_t) * WIN_WIDTH * WIN_HEIGHT * 3);
    SDL_UpdateTexture(gui->texture, NULL, gui->buffer, 0);

    gui->running = TRUE;

    SDL_Log("Initialized");
}

void gui_quit(gui_t* gui) {
    // SDL_DestroyWindow(gui->win);
    SDL_DestroyRenderer(gui->ren);
    SDL_DestroyTexture(gui->texture);

    SDL_Quit();

    SDL_Log("Quit");
}

void gui_poll_events(gui_t* gui, uint16_t* keys_state) {
    while (SDL_PollEvent(&gui->event) != 0) {
        switch (gui->event.type) {
            case SDL_QUIT:
                gui->running = FALSE;
                break;
            case SDL_KEYDOWN:
                if (gui->event.key.keysym.sym == SDLK_ESCAPE) {
                    gui->running = FALSE;
                } else {
                    BIT_SET(*keys_state, get_key(gui->event.key.keysym.sym));
                }
                break;
            case SDL_KEYUP:
                BIT_CLEAR(*keys_state, get_key(gui->event.key.keysym.sym));
                break;
            default:
                break;
        }
    }
}

void gui_set_buffer(gui_t* gui, uint8_t* buffer) {
    for (size_t i = 0; i < WIN_WIDTH * WIN_HEIGHT; ++i) {
        if (buffer[i] == 1) {
            gui->buffer[i * 3 + 0] = 205;
            gui->buffer[i * 3 + 1] = 214;
            gui->buffer[i * 3 + 2] = 244;
        } else {
            gui->buffer[i * 3 + 0] = 24;
            gui->buffer[i * 3 + 1] = 24;
            gui->buffer[i * 3 + 2] = 37;
        }
    }

    SDL_UpdateTexture(gui->texture, NULL, gui->buffer, WIN_WIDTH * 3);
}

void gui_render(gui_t* gui) {
    SDL_RenderClear(gui->ren);
    SDL_RenderCopy(gui->ren, gui->texture, NULL, NULL);
    SDL_RenderPresent(gui->ren);
}