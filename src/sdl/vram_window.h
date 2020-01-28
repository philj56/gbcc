/*
 * Copyright (C) 2017-2020 Philip Jones
 *
 * Licensed under the MIT License.
 * See either the LICENSE file, or:
 *
 * https://opensource.org/licenses/MIT
 *
 */

#ifndef GBCC_SDL_VRAM_WINDOW_H
#define GBCC_SDL_VRAM_WINDOW_H

#include "sdl.h"

void gbcc_sdl_vram_window_initialise(struct gbcc_sdl *sdl);
void gbcc_sdl_vram_window_destroy(struct gbcc_sdl *sdl);
void gbcc_sdl_vram_window_update(struct gbcc_sdl *sdl);

#endif /* GBCC_SDL_VRAM_WINDOW_H */
