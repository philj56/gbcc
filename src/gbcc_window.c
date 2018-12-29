#include "gbcc.h"
#include "gbcc_colour.h"
#include "gbcc_debug.h"
#include "gbcc_input.h"
#include "gbcc_memory.h"
#include "gbcc_time.h"
#include "gbcc_window.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

void gbcc_window_initialise(struct gbcc_window *win, struct gbc *gbc)
{
	win->gbc = gbc;

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
		gbcc_log_error("Failed to initialize SDL: %s\n", SDL_GetError());
	}
	
	win->window = SDL_CreateWindow(
			"GBCC",                    // window title
			SDL_WINDOWPOS_UNDEFINED,   // initial x position
			SDL_WINDOWPOS_UNDEFINED,   // initial y position
			GBC_SCREEN_WIDTH,      // width, in pixels
			GBC_SCREEN_HEIGHT,     // height, in pixels
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE // flags
			);

	if (win->window == NULL) {
		gbcc_log_error("Could not create window: %s\n", SDL_GetError());
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	win->renderer = SDL_CreateRenderer(
			win->window,
			-1,
			SDL_RENDERER_ACCELERATED
			);

	if (win->renderer == NULL) {
		gbcc_log_error("Could not create renderer: %s\n", SDL_GetError());
		SDL_DestroyWindow(win->window);
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	win->texture = SDL_CreateTexture(
			win->renderer,
			SDL_PIXELFORMAT_RGB888,
			SDL_TEXTUREACCESS_STATIC,
			GBC_SCREEN_WIDTH, GBC_SCREEN_HEIGHT
			);

	if (win->texture == NULL) {
		gbcc_log_error("Could not create texture: %s\n", SDL_GetError());
		SDL_DestroyRenderer(win->renderer);
		SDL_DestroyWindow(win->window);
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	SDL_RenderSetLogicalSize(
			win->renderer,
			GBC_SCREEN_WIDTH, GBC_SCREEN_HEIGHT
			);
	SDL_RenderSetIntegerScale(win->renderer, true);

	for (size_t i = 0; i < GBC_SCREEN_SIZE; i++) {
		win->buffer[i] = 0;
	}
}

void gbcc_window_destroy(struct gbcc_window *win)
{
	SDL_DestroyTexture(win->texture);
	SDL_DestroyRenderer(win->renderer);
	SDL_DestroyWindow(win->window);
}

void gbcc_window_update(struct gbcc_window *win)
{
	int err;
	uint32_t *screen = win->gbc->memory.sdl_screen;

	if (win->gbc->mode == GBC || !win->gbc->palette.precorrected) {
		for (size_t i = 0; i < GBC_SCREEN_SIZE; i++) {
			uint8_t r = (uint8_t)(screen[i] >> 19u) & 0x1Fu;
			uint8_t g = (uint8_t)(screen[i] >> 11u) & 0x1Fu;
			uint8_t b = (uint8_t)(screen[i] >> 3u) & 0x1Fu;
			win->buffer[i] = gbcc_lerp_colour(r, g, b);
		}
	} else {
		for (size_t i = 0; i < GBC_SCREEN_SIZE; i++) {
			win->buffer[i] = screen[i];
		}
	}
	/* Draw the background */
	err = SDL_UpdateTexture(
			win->texture,
			NULL,
			win->buffer,
			GBC_SCREEN_WIDTH * sizeof(win->buffer[0])
			);
	if (err < 0) {
		gbcc_log_error("Error updating texture: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	err = SDL_RenderClear(win->renderer);
	if (err < 0) {
		gbcc_log_error("Error clearing renderer: %s\n", SDL_GetError());
		if (win->renderer == NULL) {
			gbcc_log_error("NULL Renderer!\n");
		}
		exit(EXIT_FAILURE);
	}

	err = SDL_RenderCopy(win->renderer, win->texture, NULL, NULL);
	if (err < 0) {
		gbcc_log_error("Error copying texture: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	SDL_RenderPresent(win->renderer);
}
