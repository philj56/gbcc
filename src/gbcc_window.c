#include "gbcc.h"
#include "gbcc_debug.h"
#include "gbcc_input.h"
#include "gbcc_memory.h"
#include "gbcc_window.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

static int gbcc_window_thread_function(void *window);

struct gbcc_window *gbcc_window_initialise(struct gbc *gbc)
{
	SDL_Init(0);

	struct gbcc_window *win = malloc(sizeof(struct gbcc_window));
	win->gbc = gbc;
	win->quit = false;

	SDL_InitSubSystem(SDL_INIT_VIDEO);
	win->mutex = SDL_CreateMutex();
	if (win->mutex == NULL) {
		gbcc_log(GBCC_LOG_ERROR, "Could not create mutex: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	win->thread = SDL_CreateThread(
			gbcc_window_thread_function, 
			"RenderingThread", 
			(void *)(win));

	if (win->thread == NULL) {
		gbcc_log(GBCC_LOG_ERROR, "Error creating rendering thread: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	return win;
}

void gbcc_window_destroy(struct gbcc_window *win)
{
	win->quit = true;
	SDL_WaitThread(win->thread, NULL);
	SDL_DestroyMutex(win->mutex);
	SDL_DestroyTexture(win->texture);
	SDL_DestroyRenderer(win->renderer);
	SDL_DestroyWindow(win->window);
}

static int gbcc_window_thread_function(void *window)
{
	int err;
	struct gbcc_window *win = (struct gbcc_window *)window;


	win->window = SDL_CreateWindow(
			"GBCC",                    // window title
			SDL_WINDOWPOS_UNDEFINED,   // initial x position
			SDL_WINDOWPOS_UNDEFINED,   // initial y position
			GBC_SCREEN_WIDTH,      // width, in pixels
			GBC_SCREEN_HEIGHT,     // height, in pixels
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE // flags - see below
			);

	if (win->window == NULL) {
		gbcc_log(GBCC_LOG_ERROR, "Could not create window: %s\n", SDL_GetError());
		SDL_DestroyMutex(win->mutex);
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	win->renderer = SDL_CreateRenderer(
			win->window,
			-1,
			SDL_RENDERER_ACCELERATED
			);

	if (win->renderer == NULL) {
		gbcc_log(GBCC_LOG_ERROR, "Could not create renderer: %s\n", SDL_GetError());
		SDL_DestroyWindow(win->window);
		SDL_DestroyMutex(win->mutex);
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	win->texture = SDL_CreateTexture(
			win->renderer,
			SDL_PIXELFORMAT_RGB888,
			SDL_TEXTUREACCESS_STREAMING,
			GBC_SCREEN_WIDTH, GBC_SCREEN_HEIGHT
			);

	if (win->texture == NULL) {
		gbcc_log(GBCC_LOG_ERROR, "Could not create texture: %s\n", SDL_GetError());
		SDL_DestroyRenderer(win->renderer);
		SDL_DestroyWindow(win->window);
		SDL_DestroyMutex(win->mutex);
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	SDL_RenderSetLogicalSize(
			win->renderer,
			GBC_SCREEN_WIDTH, GBC_SCREEN_HEIGHT
			);

	for (size_t i = 0; i < GBC_SCREEN_SIZE; i++) {
		win->buffer[i] = 0;
	}

	/* Main rendering loop */
	while (!(win->quit)) {
		gbcc_input_process_all(win->gbc);
		if (SDL_LockMutex(win->mutex) < 0) {
			gbcc_log(GBCC_LOG_ERROR, "Could not lock mutex: %s\n", SDL_GetError());
		}
		/* Do the actual drawing */
		for (size_t i = 0; i < GBC_SCREEN_SIZE; i++) {
			win->buffer[i] = win->gbc->memory.sdl_screen[i];
		}
		/* Draw the background */
		if (SDL_UnlockMutex(win->mutex) < 0) {
			gbcc_log(GBCC_LOG_ERROR, "Could not unlock mutex: %s\n", SDL_GetError());
		}
		err = SDL_UpdateTexture(
				win->texture, 
				NULL, 
				win->buffer, 
				GBC_SCREEN_WIDTH * sizeof(win->buffer[0])
				);
		if (err < 0) {
			gbcc_log(GBCC_LOG_ERROR, "Error updating texture: %s\n", SDL_GetError());
			exit(EXIT_FAILURE);
		}

		err = SDL_RenderClear(win->renderer);
		if (err < 0) {
			gbcc_log(GBCC_LOG_ERROR, "Error clearing renderer: %s\n", SDL_GetError());
			if (win->renderer == NULL) {
				printf("NULL Renderer!\n");
			}
			exit(EXIT_FAILURE);
		}

		err = SDL_RenderCopy(win->renderer, win->texture, NULL, NULL);
		if (err < 0) {
			gbcc_log(GBCC_LOG_ERROR, "Error copying texture: %s\n", SDL_GetError());
			exit(EXIT_FAILURE);
		}

		SDL_RenderPresent(win->renderer);

		SDL_Delay(16);
	}
	printf("QUIT SDL\n");
	return 0;
}
