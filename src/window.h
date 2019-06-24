#ifndef GBCC_WINDOW_H
#define GBCC_WINDOW_H

#include "gbcc.h"
#include "constants.h"
#include "fontmap.h"
#include "vram_window.h"
#include <epoxy/gl.h>
#include <SDL2/SDL.h>
#include <stdint.h>

#define MSG_BUF_SIZE 128

struct shader {
	char *name;
	GLuint program;
};

struct fps_counter {
	uint64_t last_frame;
	struct timespec last_time;
	uint8_t last_ly;
	float fps;
	float previous[4];
	int idx;
	bool show;
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
		SDL_GLContext context;
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
	struct fps_counter fps;
	struct {
		char text[MSG_BUF_SIZE];
		int lines;
		int64_t time_left;
	} msg;
	struct gbcc_vram_window vram;
	bool screenshot;
	bool raw_screenshot;
	bool fractional_scaling;
	bool vram_display;
	bool initialised;
};

void gbcc_window_initialise(struct gbcc_window *win, struct gbc *gbc);
void gbcc_window_destroy(struct gbcc_window *win);
void gbcc_window_update(struct gbcc_window *win);
void gbcc_window_show_message(struct gbcc_window *win, const char *msg, int seconds, bool pad);
void gbcc_window_use_shader(struct gbcc_window *win, const char *name);
void gbcc_load_shader(GLuint shader, const char *filename);
GLuint gbcc_create_shader_program(const char *vert, const char *frag);

#endif /* GBCC_WINDOW_H */
