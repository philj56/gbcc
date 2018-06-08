#include "gbcc_memory.h"
#include "gbcc_window.h"
#include <stdio.h>
#include <stdlib.h>

#define BACKGROUND_MAP_START 0x9800u

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
		fprintf(stderr, "Could not create mutex: %s\n", SDL_GetError());
		exit(1);
	}

	gbcc_window_thread = SDL_CreateThread(
			gbcc_window_thread_function, 
			"RenderingThread", 
			(void *)(&gbcc_window));

	if (gbcc_window_thread == NULL) {
		fprintf(stderr, "Error creating rendering thread: %s\n", SDL_GetError());
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
	SDL_Quit();
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
			4 * GBC_SCREEN_WIDTH,      // width, in pixels
			4 * GBC_SCREEN_HEIGHT,     // height, in pixels
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE // flags - see below
			);

	if (win->window == NULL) {
		fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
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
		fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
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
		fprintf(stderr, "Could not create texture: %s\n", SDL_GetError());
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
		if (SDL_LockMutex(win->mutex) < 0) {
			fprintf(stderr, "Could not lock mutex: %s\n", SDL_GetError());
		}
		/* Do the actual drawing */
		for (size_t i = 0; i < GBC_SCREEN_SIZE; i++) {
			win->buffer[i] = 0xFFFFFF;
		}
		/* Draw the background */
		gbcc_window_draw_background(win);
		if (SDL_UnlockMutex(win->mutex) < 0) {
			fprintf(stderr, "Could not unlock mutex: %s\n", SDL_GetError());
		}
		err = SDL_UpdateTexture(
				win->texture, 
				NULL, 
				win->buffer, 
				GBC_SCREEN_WIDTH * sizeof(win->buffer[0])
				);
		if (err < 0) {
			fprintf(stderr, "Error updating texture: %s\n", SDL_GetError());
			exit(1);
		}

		err = SDL_RenderClear(win->renderer);
		if (err < 0) {
			fprintf(stderr, "Error clearing renderer: %s\n", SDL_GetError());
			if (win->renderer == NULL) {
				printf("NULL Renderer!\n");
			}
			exit(1);
		}

		err = SDL_RenderCopy(win->renderer, win->texture, NULL, NULL);
		if (err < 0) {
			fprintf(stderr, "Error copying texture: %s\n", SDL_GetError());
			exit(1);
		}

		SDL_RenderPresent(win->renderer);

		SDL_Delay(16);
	}
	return 0;
}

uint32_t n2c(struct gbc *gbc, uint8_t n)
{
	uint8_t palette = gbcc_memory_read(gbc, BGP);
	//printf("palette: %02X\n", palette);
	uint8_t colors[4] = {
		(palette & 0x03u) >> 0u,
		(palette & 0x0Cu) >> 2u,
		(palette & 0x30u) >> 4u,
		(palette & 0xC0u) >> 6u
	};
	switch (colors[n]) {
		case 3:
			return 0x000000u;
		case 2:
			return 0x555555u;
		case 1:
			return 0xAAAAAAu;
		case 0:
			return 0xFFFFFFu;
		default:
			return 0;
	}
}

void gbcc_window_draw_background(struct gbcc_window *win)
{
	uint8_t scy = gbcc_memory_read(win->gbc, SCY);
	uint8_t scx = gbcc_memory_read(win->gbc, SCX);
	
	uint8_t sty = scy / 8u;
	uint8_t stx = scx / 8u;

	for (uint8_t ty = 0; ty < GBC_SCREEN_HEIGHT / 8; ty++) {
		for (uint8_t tx = 0; tx < GBC_SCREEN_WIDTH / 8; tx++) {
			uint16_t base_addr = VRAM_START + gbcc_memory_read(win->gbc, BACKGROUND_MAP_START + 32 * ((sty + ty) % 32) + (stx + tx) % 32);
			uint8_t y = 0;
			for (uint8_t i = 0; i < 16; i++) {
				uint8_t x = 0;
				uint8_t byte = gbcc_memory_read(win->gbc, base_addr + i++); 
				win->buffer[GBC_SCREEN_WIDTH * (y + 8 * ty) + (x++ + 8 * tx)] = n2c(win->gbc, (byte & 0xC0u) >> 6u);
				win->buffer[GBC_SCREEN_WIDTH * (y + 8 * ty) + (x++ + 8 * tx)] = n2c(win->gbc, (byte & 0x30u) >> 4u);
				win->buffer[GBC_SCREEN_WIDTH * (y + 8 * ty) + (x++ + 8 * tx)] = n2c(win->gbc, (byte & 0x0Cu) >> 2u);
				win->buffer[GBC_SCREEN_WIDTH * (y + 8 * ty) + (x++ + 8 * tx)] = n2c(win->gbc, (byte & 0x03u) >> 0u);
				byte = gbcc_memory_read(win->gbc, base_addr + i); 
				win->buffer[GBC_SCREEN_WIDTH * (y + 8 * ty) + (x++ + 8 * tx)] = n2c(win->gbc, (byte & 0xC0u) >> 6u);
				win->buffer[GBC_SCREEN_WIDTH * (y + 8 * ty) + (x++ + 8 * tx)] = n2c(win->gbc, (byte & 0x30u) >> 4u);
				win->buffer[GBC_SCREEN_WIDTH * (y + 8 * ty) + (x++ + 8 * tx)] = n2c(win->gbc, (byte & 0x0Cu) >> 2u);
				win->buffer[GBC_SCREEN_WIDTH * (y + 8 * ty) + (x++ + 8 * tx)] = n2c(win->gbc, (byte & 0x03u) >> 0u);
				y++;
			}
		}
	}
}
