#ifndef GBCC_WINDOW_H
#define GBCC_WINDOW_H

#include "gbcc.h"
#include "gbcc_constants.h"
#include "gbcc_fontmap.h"
#include <SDL2/SDL.h>
#include <stdint.h>

struct gbcc_window {
	struct gbc *gbc;
	struct gbcc_fontmap font;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	uint32_t buffer[GBC_SCREEN_WIDTH * GBC_SCREEN_HEIGHT];
	struct {
		uint64_t last_frame;
		struct timespec last_time;
		uint8_t last_ly;
		float fps;
		bool show;
	} fps_counter;
	struct {
		const char *text;
		int64_t time_left;
	} msg;
	bool screenshot;
};

void gbcc_window_initialise(struct gbcc_window *win, struct gbc *gbc);
void gbcc_window_destroy(struct gbcc_window *win);
void gbcc_window_update(struct gbcc_window *win);
void gbcc_window_show_message(struct gbcc_window *win, const char *msg, int seconds);

#endif /* GBCC_WINDOW_H */
