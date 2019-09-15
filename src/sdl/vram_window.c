#include "../gbcc.h"
#include "../debug.h"
#include "../input.h"
#include "../nelem.h"
#include "../vram_window.h"
#include "sdl.h"
#include "vram_window.h"
#include <math.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

void gbcc_sdl_vram_window_initialise(struct gbcc_sdl *sdl)
{
	/* Main OpenGL settings */
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);


	sdl->vram_window = SDL_CreateWindow(
			"GBCC VRAM Data",                    // window title
			SDL_WINDOWPOS_UNDEFINED,   // initial x position
			SDL_WINDOWPOS_UNDEFINED,   // initial y position
			VRAM_WINDOW_WIDTH,      // width, in pixels
			VRAM_WINDOW_HEIGHT,     // height, in pixels
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE // flags
			);

	if (sdl->vram_window == NULL) {
		gbcc_log_error("Could not create vram window: %s\n", SDL_GetError());
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	sdl->vram_context = SDL_GL_CreateContext(sdl->vram_window);
	SDL_GL_MakeCurrent(sdl->vram_window, sdl->vram_context);

	gbcc_vram_window_initialise(&sdl->gbc);
}

void gbcc_sdl_vram_window_destroy(struct gbcc_sdl *sdl)
{
	SDL_GL_MakeCurrent(sdl->vram_window, sdl->vram_context);
	gbcc_vram_window_deinitialise(&sdl->gbc);
	SDL_GL_DeleteContext(sdl->context);
	SDL_DestroyWindow(sdl->window);
}

void gbcc_sdl_vram_window_update(struct gbcc_sdl *sdl)
{
	SDL_GL_MakeCurrent(sdl->vram_window, sdl->vram_context);
	SDL_GL_GetDrawableSize(sdl->vram_window, &sdl->gbc.vram_window.width, &sdl->gbc.vram_window.height);
	gbcc_vram_window_update(&sdl->gbc);
	SDL_GL_SwapWindow(sdl->window);
}
