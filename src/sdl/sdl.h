/*
 * Copyright (C) 2017-2020 Philip Jones
 *
 * Licensed under the MIT License.
 * See either the LICENSE file, or:
 *
 * https://opensource.org/licenses/MIT
 *
 */

#ifndef GBCC_SDL_H
#define GBCC_SDL_H

#include "../gbcc.h"
#include <SDL2/SDL.h>
#include <stdbool.h>

struct gbcc_sdl {
	struct gbcc gbc;
	bool fullscreen;
	SDL_Window *window;
	SDL_Window *vram_window;
	SDL_GLContext context;
	SDL_GLContext vram_context;
	SDL_GameController *game_controller;
	SDL_Haptic *haptic;
};

void gbcc_sdl_initialise(struct gbcc_sdl *sdl);
void gbcc_sdl_destroy(struct gbcc_sdl *sdl);
void gbcc_sdl_update(struct gbcc_sdl *sdl);
void gbcc_sdl_process_input(struct gbcc_sdl *sdl);

#endif /* GBCC_SDL_H */
