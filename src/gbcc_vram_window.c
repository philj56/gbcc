#include "gbcc.h"
#include "gbcc_bit_utils.h"
#include "gbcc_debug.h"
#include "gbcc_input.h"
#include "gbcc_memory.h"
#include "gbcc_vram_window.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define VRAM_WINDOW_WIDTH_TILES (VRAM_WINDOW_WIDTH / 8)
#define VRAM_WINDOW_HEIGHT_TILES (VRAM_WINDOW_HEIGHT / 8)

static int window_thread_function(void *window);

struct gbcc_vram_window *gbcc_vram_window_initialise(struct gbc *gbc)
{
	SDL_Init(0);

	struct gbcc_vram_window *win = malloc(sizeof(struct gbcc_vram_window));
	win->gbc = gbc;
	win->quit = false;

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
		gbcc_log(GBCC_LOG_ERROR, "Failed to initialize SDL: %s\n", SDL_GetError());
	}

	win->thread = SDL_CreateThread(
			window_thread_function, 
			"VramThread", 
			(void *)(win));

	if (win->thread == NULL) {
		gbcc_log(GBCC_LOG_ERROR, "Error creating rendering thread: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	return win;
}

void gbcc_vram_window_destroy(struct gbcc_vram_window *win)
{
	win->quit = true;
	SDL_WaitThread(win->thread, NULL);
	SDL_DestroyTexture(win->texture);
	SDL_DestroyRenderer(win->renderer);
	SDL_DestroyWindow(win->window);
}

static int window_thread_function(void *window)
{
	int err;
	struct gbcc_vram_window *win = (struct gbcc_vram_window *)window;


	SDL_Delay(100);
	win->window = SDL_CreateWindow(
			"GBCC VRAM data",          // window title
			SDL_WINDOWPOS_UNDEFINED,   // initial x position
			SDL_WINDOWPOS_UNDEFINED,   // initial y position
			VRAM_WINDOW_WIDTH,      // width, in pixels
			VRAM_WINDOW_HEIGHT,     // height, in pixels
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE // flags - see below
			);

	if (win->window == NULL) {
		gbcc_log(GBCC_LOG_ERROR, "Could not create window: %s\n", SDL_GetError());
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
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	win->texture = SDL_CreateTexture(
			win->renderer,
			SDL_PIXELFORMAT_RGB888,
			SDL_TEXTUREACCESS_STREAMING,
			VRAM_WINDOW_WIDTH, VRAM_WINDOW_HEIGHT
			);

	if (win->texture == NULL) {
		gbcc_log(GBCC_LOG_ERROR, "Could not create texture: %s\n", SDL_GetError());
		SDL_DestroyRenderer(win->renderer);
		SDL_DestroyWindow(win->window);
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	SDL_RenderSetLogicalSize(
			win->renderer,
			VRAM_WINDOW_WIDTH, VRAM_WINDOW_HEIGHT
			);

	for (size_t i = 0; i < VRAM_WINDOW_SIZE; i++) {
		win->buffer[i] = 0;
	}

	/* Main rendering loop */
	while (!(win->quit)) {
		/* Do the actual drawing */
		for (int j = 0; j < VRAM_WINDOW_HEIGHT_TILES / 2; j++) {
			for (int i = 0; i < VRAM_WINDOW_WIDTH_TILES; i++) {
				for (int y = 0; y < 8; y++) {
					uint8_t lo = win->gbc->memory.vram_bank[0][16 * (j * VRAM_WINDOW_WIDTH_TILES + i) + 2*y];
					uint8_t hi = win->gbc->memory.vram_bank[0][16 * (j * VRAM_WINDOW_WIDTH_TILES + i) + 2*y + 1];
					for (int x = 0; x < 8; x++) {
						uint8_t colour = (uint8_t)(check_bit(hi, 7 - x) << 1u) | check_bit(lo, 7 - x);
						uint32_t p = 0;
						switch (colour) {
							case 3:
								p = 0x000000u;
								break;
							case 2:
								p = 0x6b7353u;
								break;
							case 1:
								p = 0x8b956du;
								break;
							case 0:
								p = 0xc4cfa1u;
								break;
						}
						win->buffer[8 * (VRAM_WINDOW_WIDTH * j + i) + VRAM_WINDOW_WIDTH * y + x] = p;
					}
				}
			}
		}
		
		for (size_t j = 0; j < VRAM_WINDOW_HEIGHT_TILES / 2; j++) {
			for (size_t i = 0; i < VRAM_WINDOW_WIDTH_TILES; i++) {
				for (int y = 0; y < 8; y++) {
					uint8_t lo = win->gbc->memory.vram_bank[1][16 * (j * VRAM_WINDOW_WIDTH_TILES + i) + 2*y];
					uint8_t hi = win->gbc->memory.vram_bank[1][16 * (j * VRAM_WINDOW_WIDTH_TILES + i) + 2*y + 1];
					for (int x = 0; x < 8; x++) {
						uint8_t colour = (uint8_t)(check_bit(hi, 7 - x) << 1u) | check_bit(lo, 7 - x);
						uint32_t p = 0;
						switch (colour) {
							case 3:
								p = 0x000000u;
								break;
							case 2:
								p = 0x6b7353u;
								break;
							case 1:
								p = 0x8b956du;
								break;
							case 0:
								p = 0xc4cfa1u;
								break;
						}
						win->buffer[8 * (VRAM_WINDOW_WIDTH * (j + VRAM_WINDOW_HEIGHT_TILES / 2) + i) + VRAM_WINDOW_WIDTH * y + x] = p;
					}
				}
			}
		}
		/* Draw the background */
		err = SDL_UpdateTexture(
				win->texture, 
				NULL, 
				win->buffer, 
				VRAM_WINDOW_WIDTH * sizeof(win->buffer[0])
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
