#include "gbcc.h"
#include "gbcc_debug.h"
#include "gbcc_input.h"
#include "gbcc_memory.h"
#include "gbcc_window.h"
#include <stdio.h>
#include <stdlib.h>


static int gbcc_window_thread_function(void *window);

static SDL_Thread *gbcc_window_thread;
static struct gbcc_window gbcc_window;

void gbcc_window_initialise(struct gbc *gbc)
{
	SDL_Init(0);

	gbcc_window.gbc = gbc;
	gbcc_window.quit = false;

	gbcc_window.mutex = SDL_CreateMutex();
	if (gbcc_window.mutex == NULL) {
		gbcc_log(GBCC_LOG_ERROR, "Could not create mutex: %s\n", SDL_GetError());
		exit(1);
	}

	gbcc_window_thread = SDL_CreateThread(
			gbcc_window_thread_function, 
			"RenderingThread", 
			(void *)(&gbcc_window));

	if (gbcc_window_thread == NULL) {
		gbcc_log(GBCC_LOG_ERROR, "Error creating rendering thread: %s\n", SDL_GetError());
		exit(1);
	}
}

void gbcc_window_quit()
{
	gbcc_window.quit = true;
	SDL_WaitThread(gbcc_window_thread, NULL);
	SDL_DestroyMutex(gbcc_window.mutex);
	SDL_DestroyTexture(gbcc_window.texture);
	SDL_DestroyRenderer(gbcc_window.renderer);
	SDL_DestroyWindow(gbcc_window.window);
	atexit(SDL_Quit);
}

static int gbcc_window_thread_function(void *window)
{
	int err;
	struct gbcc_window *win = (struct gbcc_window *)window;

	SDL_InitSubSystem(SDL_INIT_VIDEO);

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
		exit(1);
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
		exit(1);
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
		exit(1);
	}

	SDL_RenderSetLogicalSize(
			win->renderer,
			GBC_SCREEN_WIDTH, GBC_SCREEN_HEIGHT
			);

	for (size_t i = 0; i < GBC_SCREEN_SIZE; i++) {
		win->buffer[i] = 0;
	}

	atexit(gbcc_window_quit);

	/* Main rendering loop */
	while (!(win->quit)) {
		gbcc_input_process_all(win->gbc);
		if (SDL_LockMutex(win->mutex) < 0) {
			gbcc_log(GBCC_LOG_ERROR, "Could not lock mutex: %s\n", SDL_GetError());
		}
		/* Do the actual drawing */
		for (size_t i = 0; i < GBC_SCREEN_SIZE; i++) {
			win->buffer[i] = win->gbc->memory.screen[i / GBC_SCREEN_WIDTH][i % GBC_SCREEN_WIDTH];
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
			exit(1);
		}

		err = SDL_RenderClear(win->renderer);
		if (err < 0) {
			gbcc_log(GBCC_LOG_ERROR, "Error clearing renderer: %s\n", SDL_GetError());
			if (win->renderer == NULL) {
				printf("NULL Renderer!\n");
			}
			exit(1);
		}

		err = SDL_RenderCopy(win->renderer, win->texture, NULL, NULL);
		if (err < 0) {
			gbcc_log(GBCC_LOG_ERROR, "Error copying texture: %s\n", SDL_GetError());
			exit(1);
		}

		SDL_RenderPresent(win->renderer);

		SDL_Delay(16);
	}
	printf("QUIT SDL\n");
	return 0;
}
