#ifndef GBCC_WINDOW_H
#define GBCC_WINDOW_H

#include "gbcc.h"
#include "constants.h"
#include "fontmap.h"
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <stdint.h>

#define MSG_BUF_SIZE 128

struct shader {
	char *name;
	GLuint program;
};

struct gbcc_window {
	struct gbc *gbc;
	struct gbcc_fontmap font;
	SDL_Window *window;
	uint32_t width;
	uint32_t height;
	uint32_t x;
	uint32_t y;
	uint32_t buffer[GBC_SCREEN_SIZE];
	struct {
		GLuint vbo;
		GLuint vao;
		GLuint ebo;
		GLuint fbo;
		GLuint fbo_texture;
		GLuint rbo;
		GLuint texture;
		GLuint base_shader;
		int cur_shader;
		struct shader shaders[3];
	} gl;
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
	bool fractional_scaling;
};

void gbcc_window_initialise(struct gbcc_window *win, struct gbc *gbc);
void gbcc_window_destroy(struct gbcc_window *win);
void gbcc_window_update(struct gbcc_window *win);
void gbcc_window_show_message(struct gbcc_window *win, const char *msg, int seconds);

#endif /* GBCC_WINDOW_H */
