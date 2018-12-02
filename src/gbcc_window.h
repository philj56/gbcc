#ifndef GBCC_WINDOW_H
#define GBCC_WINDOW_H

#include "gbcc.h"
#include "gbcc_constants.h"
#include <SDL2/SDL.h>

struct gbcc_window {
	struct gbc *gbc;
	SDL_Thread *thread;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	uint32_t buffer[GBC_SCREEN_WIDTH * GBC_SCREEN_HEIGHT];
	bool quit;
	bool vsync;
};

struct gbcc_window *gbcc_window_initialise(struct gbc *gbc, bool vsync);
void gbcc_window_destroy(struct gbcc_window *win);

#endif /* GBCC_WINDOW_H */
