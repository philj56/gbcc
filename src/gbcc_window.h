#ifndef GBCC_WINDOW_H
#define GBCC_WINDOW_H

#include "gbcc.h"
#include "gbcc_constants.h"
#include "gbcc_fontmap.h"
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <stdint.h>

#define MSG_BUF_SIZE 128

#define FBO_WIDTH (7 * GBC_SCREEN_WIDTH)
#define FBO_HEIGHT (7 * GBC_SCREEN_HEIGHT)
#define FBO_SIZE (FBO_WIDTH * FBO_HEIGHT)

struct shader {
	char *name;
	GLuint program;
};

struct gbcc_window {
	struct gbc *gbc;
	struct gbcc_fontmap font;
	SDL_Window *window;
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
		struct shader shaders[2];
	} gl;
	struct {
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

void gbcc_window_initialise(struct gbcc_window *win, struct gbc *gbc);
void gbcc_window_destroy(struct gbcc_window *win);
void gbcc_window_update(struct gbcc_window *win);
void gbcc_window_show_message(struct gbcc_window *win, const char *msg, int seconds);

#endif /* GBCC_WINDOW_H */
