#ifndef GBCC_PLATFORM_H
#define GBCC_PLATFORM_H

#include <SDL2/SDL.h>

struct gbcc_platform {
	SDL_Window *window;
	SDL_GLContext context;
	SDL_GameController *game_controller;
	SDL_Haptic *haptic;
};

#endif /* GBCC_PLATFORM_H */
