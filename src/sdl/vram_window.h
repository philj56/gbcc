#ifndef GBCC_SDL_VRAM_WINDOW_H
#define GBCC_SDL_VRAM_WINDOW_H

#include "../gbcc.h"

void gbcc_sdl_vram_window_initialise(struct gbcc *gbc);
void gbcc_sdl_vram_window_destroy(struct gbcc *gbc);
void gbcc_sdl_vram_window_update(struct gbcc *gbc);

#endif /* GBCC_SDL_VRAM_WINDOW_H */
