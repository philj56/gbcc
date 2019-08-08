#ifndef GBCC_SDL_H
#define GBCC_SDL_H

#include "../gbcc.h"

void gbcc_sdl_initialise(struct gbcc *gbc);
void gbcc_sdl_destroy(struct gbcc *gbc);
void gbcc_sdl_update(struct gbcc *gbc);
void gbcc_sdl_process_input(struct gbcc *gbc);

#endif /* GBCC_SDL_H */
