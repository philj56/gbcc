#ifndef GBCC_WINDOW_H
#define GBCC_WINDOW_H

#include "gbcc.h"
#include "gbcc_constants.h"
#include "gbcc_fontmap.h"
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <stdint.h>

#define MSG_BUF_SIZE 128

enum scaling_type { SCALING_NONE, SCALING_SUBPIXEL };

struct gbcc_window {
	struct gbc *gbc;
	struct gbcc_fontmap font;
	SDL_Window *window;
	SDL_Renderer *renderer;
	GLuint texture;
	uint32_t *buffer;
	struct {
		enum scaling_type type;
		uint32_t factor;
	} scaling;
	struct {
		uint64_t last_frame;
		struct timespec last_time;
		uint8_t last_ly;
		float fps;
		bool show;
	} fps_counter;
	struct {
		char text[MSG_BUF_SIZE];
		int lines;
		int64_t time_left;
	} msg;
	bool screenshot;
	bool raw_screenshot;
};

void gbcc_window_initialise(struct gbcc_window *win, struct gbc *gbc, enum scaling_type scaling);
void gbcc_window_destroy(struct gbcc_window *win);
void gbcc_window_update(struct gbcc_window *win);
void gbcc_window_show_message(struct gbcc_window *win, const char *msg, int seconds);

#endif /* GBCC_WINDOW_H */
