#include "../gbcc.h"
#include "../debug.h"
#include "../input.h"
#include "../nelem.h"
#include "../vram_window.h"
#include "sdl.h"
#include <math.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

void gbcc_sdl_vram_window_initialise(struct gbcc *gbc)
{
	/* Main OpenGL settings */
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);


	gbc->vram_window.platform.window = SDL_CreateWindow(
			"GBCC VRAM Data",                    // window title
			SDL_WINDOWPOS_UNDEFINED,   // initial x position
			SDL_WINDOWPOS_UNDEFINED,   // initial y position
			VRAM_WINDOW_WIDTH,      // width, in pixels
			VRAM_WINDOW_HEIGHT,     // height, in pixels
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE // flags
			);

	if (gbc->vram_window.platform.window == NULL) {
		gbcc_log_error("Could not create vram window: %s\n", SDL_GetError());
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	gbc->vram_window.platform.context = SDL_GL_CreateContext(gbc->vram_window.platform.window);
	SDL_GL_MakeCurrent(gbc->vram_window.platform.window, gbc->vram_window.platform.context);

	gbcc_vram_window_initialise(gbc);
}

void gbcc_sdl_vram_window_destroy(struct gbcc *gbc)
{
	struct gbcc_vram_window *win = &gbc->vram_window;
	SDL_GL_MakeCurrent(win->platform.window, win->platform.context);
	gbcc_vram_window_deinitialise(gbc);
	SDL_GL_DeleteContext(win->platform.context);
	SDL_DestroyWindow(win->platform.window);
}

void gbcc_sdl_vram_window_update(struct gbcc *gbc)
{
	struct gbcc_vram_window *win = &gbc->vram_window;
	SDL_GL_MakeCurrent(win->platform.window, win->platform.context);
	SDL_GL_GetDrawableSize(win->platform.window, &win->width, &win->height);
	gbcc_vram_window_update(gbc);
	SDL_GL_SwapWindow(win->platform.window);
}
