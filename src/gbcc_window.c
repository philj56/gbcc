#include "gbcc.h"
#include "gbcc_colour.h"
#include "gbcc_constants.h"
#include "gbcc_debug.h"
#include "gbcc_fontmap.h"
#include "gbcc_input.h"
#include "gbcc_memory.h"
#include "gbcc_screenshot.h"
#include "gbcc_time.h"
#include "gbcc_window.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

static void render_text(struct gbcc_window *win, const char *text, uint8_t x, uint8_t y);
static void render_character(struct gbcc_window *win, char c, uint8_t x, uint8_t y);
static void update_text(struct gbcc_window *win);

void gbcc_window_initialise(struct gbcc_window *win, struct gbc *gbc)
{
	win->gbc = gbc;
	clock_gettime(CLOCK_REALTIME, &win->fps_counter.last_time);
	gbcc_fontmap_load(&win->font, "tileset.png");

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
	/* TODO: All this should really be somewhere else */
	if (win->screenshot) {
		win->screenshot = false;
		gbcc_screenshot(win);
	}
	update_text(win);
	if (win->fps_counter.show) {
		char fps_text[16];
		snprintf(fps_text, 16, " FPS: %.0f ", win->fps_counter.fps);
		render_text(win, fps_text, 0, 0);
	}
	if (win->msg.time_left > 0) {
		render_text(win, win->msg.text, 0, GBC_SCREEN_HEIGHT - win->font.tile_height);
	}
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

void gbcc_window_show_message(struct gbcc_window *win, const char *msg, int seconds)
{
	win->msg.text = msg;
	win->msg.time_left = seconds * 1000000000;
}

void render_text(struct gbcc_window *win, const char *text, uint8_t x, uint8_t y)
{
	for (int i = 0; text[i] != '\0'; i++) {
		render_character(win, text[i], x, y);
		x += win->font.tile_width;
		if (x > GBC_SCREEN_WIDTH - win->font.tile_width) {
			x = 0;
			y += win->font.tile_height;
			if (y > GBC_SCREEN_HEIGHT - win->font.tile_height) {
				return;
			}
		}
	}
}

void render_character(struct gbcc_window *win, char c, uint8_t x, uint8_t y)
{
	uint32_t tw = win->font.tile_width;
	uint32_t th = win->font.tile_height;
	uint32_t char_x = (c % 16) * tw;
	uint32_t char_y = (c / 16) * (16 * tw) * th;
	for (uint32_t j = 0; j < th; j++) {
		for (uint32_t i = 0; i < tw; i++) {
			uint32_t src_px = char_y + j * (16 * tw) + char_x + i;
			uint32_t dst_px = (y + j) * GBC_SCREEN_WIDTH + x + i;
			if (win->font.bitmap[src_px] > 0) {
				win->buffer[dst_px] = 0xFFFFFFu;
			} else {
				uint8_t r = (win->buffer[dst_px] >> 0x16u) & 0xFFu;
				uint8_t g = (win->buffer[dst_px] >> 0x08u) & 0xFFu;
				uint8_t b = (win->buffer[dst_px] >> 0x00u) & 0xFFu;
				uint32_t res = 0;
				r = (uint8_t)round(r / 4.0);
				g = (uint8_t)round(g / 4.0);
				b = (uint8_t)round(b / 4.0);
				res |= (uint32_t)(r << 0x16u);
				res |= (uint32_t)(g << 0x08u);
				res |= (uint32_t)(b << 0x00u);
				win->buffer[dst_px] = res;
			}
		}
	}
}

void update_text(struct gbcc_window *win)
{
	struct timespec cur_time;
	clock_gettime(CLOCK_REALTIME, &cur_time);
	double dt = gbcc_time_diff(&cur_time, &win->fps_counter.last_time);

	/* Update FPS counter */
	win->fps_counter.last_time = cur_time;
	float df = win->gbc->frame - win->fps_counter.last_frame;
	uint8_t ly = gbcc_memory_read(win->gbc, LY, true);
	if (ly < win->fps_counter.last_ly) {
		df -= 1;
		ly += (154 - win->fps_counter.last_ly);
	} else {
		ly -= win->fps_counter.last_ly;
	}
	df += ly / 154.0;
	win->fps_counter.last_frame = win->gbc->frame;
	win->fps_counter.last_ly = gbcc_memory_read(win->gbc, LY, true);
	win->fps_counter.fps = df / (dt / 1e9);

	/* Update message timer */
	if (win->msg.time_left > 0) {
		win->msg.time_left -= (int64_t)dt;
	}
}
