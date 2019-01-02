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
#include <string.h>

static void render_text(struct gbcc_window *win, const char *text, uint8_t x, uint8_t y);
static void render_character(struct gbcc_window *win, char c, uint8_t x, uint8_t y);
static void update_text(struct gbcc_window *win);
static uint32_t subpixel_lut[32][32][32][6];

static void fill_lookup()
{
	const uint32_t red = 0xFF7145;
	const uint32_t green = 0xC1D650;
	const uint32_t blue = 0x3BCEFF;
	static bool done = false;
	if (done) {
		return;
	}
	for (uint32_t r = 0; r < 32; r++) {
		for (uint32_t g = 0; g < 32; g++) {
			for (uint32_t b = 0; b < 32; b++) {
				uint32_t r2 = gbcc_add_colours(0, red, r / 32.0);
				uint32_t g2 = gbcc_add_colours(0, green, g / 32.0);
				uint32_t b2 = gbcc_add_colours(0, blue, b / 32.0);
				subpixel_lut[r][g][b][0] = gbcc_add_colours(r2, 0, 0.7);
				subpixel_lut[r][g][b][1] = gbcc_add_colours(r2, g2, 0.7);
				subpixel_lut[r][g][b][2] = gbcc_add_colours(g2, r2, 0.7);
				subpixel_lut[r][g][b][3] = gbcc_add_colours(g2, b2, 0.7);
				subpixel_lut[r][g][b][4] = gbcc_add_colours(b2, g2, 0.7);
				subpixel_lut[r][g][b][5] = gbcc_add_colours(b2, 0, 0.7);
			}
		}
	}
	done = true;
}

void gbcc_window_initialise(struct gbcc_window *win, struct gbc *gbc, enum scaling_type scaling)
{
	win->scaling.type = scaling;
	switch (scaling) {
		case SCALING_NONE:
			win->scaling.factor = 1;
			break;
		case SCALING_SUBPIXEL:
			win->scaling.factor = 7;
			break;
		default:
			gbcc_log_error("Invalid scaling type\n");
			exit(EXIT_FAILURE);
	}
	win->buffer = calloc((win->scaling.factor * win->scaling.factor) * GBC_SCREEN_SIZE, sizeof(*win->buffer));
	win->gbc = gbc;
	clock_gettime(CLOCK_REALTIME, &win->fps_counter.last_time);
	gbcc_fontmap_load(&win->font, "tileset.png");

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
		gbcc_log_error("Failed to initialize SDL: %s\n", SDL_GetError());
	}
	if (scaling == SCALING_SUBPIXEL){
		SDL_DisplayMode dm;
		SDL_GetDesktopDisplayMode(0, &dm);
		if (dm.w < 3 * GBC_SCREEN_WIDTH || dm.h < 3 * GBC_SCREEN_HEIGHT) {
			gbcc_log_warning("Minimum recommended screen size for subpixel scaling is %dx%d\n",
					3 * GBC_SCREEN_WIDTH, 3 * GBC_SCREEN_HEIGHT);
		}
	}
	
	win->window = SDL_CreateWindow(
			"GBCC",                    // window title
			SDL_WINDOWPOS_UNDEFINED,   // initial x position
			SDL_WINDOWPOS_UNDEFINED,   // initial y position
			win->scaling.factor * GBC_SCREEN_WIDTH,      // width, in pixels
			win->scaling.factor * GBC_SCREEN_HEIGHT,     // height, in pixels
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE // flags
			);

	if (win->window == NULL) {
		gbcc_log_error("Could not create window: %s\n", SDL_GetError());
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	switch (scaling) {
		case SCALING_NONE:
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
			break;
		case SCALING_SUBPIXEL:
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
			break;
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
			win->scaling.factor * GBC_SCREEN_WIDTH,
			win->scaling.factor * GBC_SCREEN_HEIGHT
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
			GBC_SCREEN_WIDTH,
			GBC_SCREEN_HEIGHT
			);
	SDL_RenderSetIntegerScale(win->renderer, true);

	for (size_t i = 0; i < win->scaling.factor * win->scaling.factor * GBC_SCREEN_SIZE; i++) {
		win->buffer[i] = 0;
	}
	fill_lookup();
}

void gbcc_window_destroy(struct gbcc_window *win)
{
	free(win->buffer);
	gbcc_fontmap_destroy(&win->font);
	SDL_DestroyTexture(win->texture);
	SDL_DestroyRenderer(win->renderer);
	SDL_DestroyWindow(win->window);
}

void gbcc_window_update(struct gbcc_window *win)
{
	int err;
	uint32_t *screen = win->gbc->memory.sdl_screen;

	switch (win->scaling.type) {
		case SCALING_NONE:
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
			break;
		case SCALING_SUBPIXEL:
			for (size_t i = 0; i < GBC_SCREEN_SIZE; i++) {
				size_t x = (i % GBC_SCREEN_WIDTH) * 7;
				size_t y = (i / GBC_SCREEN_WIDTH) * 7;
				uint8_t r = (screen[i] >> 19u) & 0x1Fu;
				uint8_t g = (screen[i] >> 11u) & 0x1Fu;
				uint8_t b = (screen[i] >> 3u) & 0x1Fu;
				uint8_t r2 = 0;
				uint8_t b2 = 0;
				if (x < 7 * (GBC_SCREEN_WIDTH - 1)) {
					r2 = (screen[i + 1] >> 19u) & 0x1Fu;
				}
				if (x > 0) {
					b2 = (screen[i - 1] >> 3u) & 0x1Fu;
				}
				for (int n = 0; n < 6; n++) {
					size_t j = (y + n) * 7 * GBC_SCREEN_WIDTH + x;
					win->buffer[j++] = subpixel_lut[r][g][b2][0];
					win->buffer[j++] = subpixel_lut[r][g][b][1];
					win->buffer[j++] = subpixel_lut[r][g][b][2];
					win->buffer[j++] = subpixel_lut[r][g][b][3];
					win->buffer[j++] = subpixel_lut[r][g][b][4];
					win->buffer[j++] = subpixel_lut[r2][g][b][5];
					win->buffer[j] = gbcc_add_colours(0, subpixel_lut[r2][g][b][0], 0.7);
					win->buffer[j] = gbcc_add_colours(win->buffer[j], subpixel_lut[r][g][b][5], 0.7);
				}
				for (int m = 0; m < 7; m++) {
					size_t j = (y + 6) * 7 * GBC_SCREEN_WIDTH + x + m;
					win->buffer[j] = 0;
				}
			}
			break;
	}
	if (win->screenshot || win->raw_screenshot) {
		gbcc_screenshot(win);
		win->screenshot = false;
		win->raw_screenshot = false;
	}
	update_text(win);
	if (win->fps_counter.show) {
		char fps_text[16];
		snprintf(fps_text, 16, " FPS: %.0f ", win->fps_counter.fps);
		render_text(win, fps_text, 0, 0);
	}
	if (win->msg.time_left > 0) {
		render_text(win, win->msg.text, 0, GBC_SCREEN_HEIGHT - win->msg.lines * win->font.tile_height);
	}
	switch (win->scaling.type) {
		case SCALING_NONE:
			err = SDL_UpdateTexture(
					win->texture,
					NULL,
					win->buffer,
					GBC_SCREEN_WIDTH * sizeof(win->buffer[0])
					);
			break;
		case SCALING_SUBPIXEL:
			err = SDL_UpdateTexture(
					win->texture,
					NULL,
					win->buffer,
					7 * GBC_SCREEN_WIDTH * sizeof(win->buffer[0])
					);
			break;
	}
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
	strncpy(win->msg.text, msg, MSG_BUF_SIZE);
	win->msg.lines = 1 + strlen(win->msg.text) / (GBC_SCREEN_WIDTH / win->font.tile_width);
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
			for (uint32_t n = 0; n < win->scaling.factor; n++) {
				for (uint32_t m = 0; m < win->scaling.factor; m++) {
					uint32_t dst_px = (n + (j + y) * win->scaling.factor) * win->scaling.factor * GBC_SCREEN_WIDTH + m + (i + x) * win->scaling.factor;
					if (win->font.bitmap[src_px] > 0) {
						win->buffer[dst_px] = 0xFFFFFFu;
					} else {
						uint8_t r = (win->buffer[dst_px] >> 16u) & 0xFFu;
						uint8_t g = (win->buffer[dst_px] >> 8u) & 0xFFu;
						uint8_t b = (win->buffer[dst_px] >> 0u) & 0xFFu;
						uint32_t res = 0;
						r = (uint8_t)round(r / 4.0);
						g = (uint8_t)round(g / 4.0);
						b = (uint8_t)round(b / 4.0);
						res |= (uint32_t)(r << 16u);
						res |= (uint32_t)(g << 8u);
						res |= (uint32_t)(b << 0u);
						win->buffer[dst_px] = res;
					}
				}
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
	uint8_t ly = gbcc_memory_read(win->gbc, LY, false);
	if (ly < win->fps_counter.last_ly) {
		df -= 1;
		ly += (154 - win->fps_counter.last_ly);
	} else {
		ly -= win->fps_counter.last_ly;
	}
	df += ly / 154.0;
	win->fps_counter.last_frame = win->gbc->frame;
	win->fps_counter.last_ly = gbcc_memory_read(win->gbc, LY, false);
	win->fps_counter.fps = df / (dt / 1e9);

	/* Update message timer */
	if (win->msg.time_left > 0) {
		win->msg.time_left -= (int64_t)dt;
	}
}
