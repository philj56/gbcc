#ifndef GBCC_VRAM_WINDOW_H
#define GBCC_VRAM_WINDOW_H

#include "gbcc.h"
#include "gbcc_constants.h"
#include <SDL2/SDL.h>

#define VRAM_WINDOW_WIDTH (8*24)
#define VRAM_WINDOW_HEIGHT (8*32)
#define VRAM_WINDOW_SIZE (VRAM_WINDOW_WIDTH * VRAM_WINDOW_HEIGHT)

struct gbcc_vram_window {
	struct gbc *gbc;
	SDL_Thread *thread;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	uint32_t buffer[VRAM_WINDOW_SIZE];
	bool quit;
};

struct gbcc_vram_window *gbcc_vram_window_initialise(struct gbc *gbc);
void gbcc_vram_window_destroy(struct gbcc_vram_window *win);

#endif /* GBCC_VRAM_WINDOW_H */
