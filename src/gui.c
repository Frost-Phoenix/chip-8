#include "gui.h"

#include <stdlib.h>
#include <string.h>

#define NB_KEYS 16


/******************************************************
 *                 Private functions                  *
 ******************************************************/

static const char keys[NB_KEYS] = {
    KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR,
    KEY_Q, KEY_W, KEY_E, KEY_R,
    KEY_A, KEY_S, KEY_D, KEY_F,
    KEY_Z, KEY_X, KEY_C, KEY_V
};


/******************************************************
 *                 Public functions                   *
 ******************************************************/

void gui_init(gui_t* gui, const char* title, int width, int height) {
    Image img;

    InitWindow(width, height, title);
    SetExitKey(KEY_ESCAPE);

    memset(gui->buffer, 0, sizeof(uint8_t) * CHIP8_DISPLAY_WIDTH * CHIP8_DISPLAY_HEIGHT * 3);

    img = (Image){
        .data = gui->buffer,
        .width = CHIP8_DISPLAY_WIDTH,
        .height = CHIP8_DISPLAY_HEIGHT,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8,
        .mipmaps = 1,
    };

    gui->texture = LoadTextureFromImage(img);
    UpdateTexture(gui->texture, gui->buffer);

    gui->source = (Rectangle){
        .x = 0,
        .y = 0,
        .width = CHIP8_DISPLAY_WIDTH,
        .height = CHIP8_DISPLAY_HEIGHT,
    };
    gui->dest = (Rectangle){
        .x = 0,
        .y = 0,
        .width = width,
        .height = height,
    };

    gui->running = TRUE;
}

void gui_quit() {
    CloseWindow();
}

void gui_poll_events(gui_t* gui, uint16_t* keys_state) {
    gui_render(gui);

    gui->running = !WindowShouldClose();

    for (size_t i = 0; i < 16; i++) {
        uint8_t key = keys[i];

        if (IsKeyDown(key)) {
            BIT_SET(*keys_state, get_key(key));
        } else {
            BIT_CLEAR(*keys_state, get_key(key));
        }
    }
}

void gui_set_buffer(gui_t* gui, uint8_t* buffer) {
    for (size_t i = 0; i < CHIP8_DISPLAY_WIDTH * CHIP8_DISPLAY_HEIGHT; ++i) {
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

    UpdateTexture(gui->texture, gui->buffer);
}

void gui_render(gui_t* gui) {
    BeginDrawing();
    ClearBackground(WHITE);
    DrawTexturePro(gui->texture, gui->source, gui->dest, (Vector2) { 0, 0 }, 0.0f, WHITE);
    EndDrawing();
}