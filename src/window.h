#ifndef GBCC_WINDOW_H
#define GBCC_WINDOW_H

#include "constants.h"
#include "fontmap.h"
#ifdef GBCC_GTK
#include "gtk/platform.h"
#else
#include "sdl/platform.h"
#endif
#include <epoxy/gl.h>
#include <stdint.h>
#include <time.h>

#define MSG_BUF_SIZE 128

struct gbcc;

struct shader {
	char *name;
	GLuint program;
};

struct fps_counter {
	uint64_t last_frame;
	struct timespec last_time;
	uint8_t last_ly;
	float fps;
	float previous[5];
	int idx;
	bool show;
};

struct gbcc_window {
	struct gbcc_fontmap font;
	uint32_t x;
	uint32_t y;
	int32_t width;
	int32_t height;
	uint32_t buffer[GBC_SCREEN_SIZE];
	uint32_t last_buffer[GBC_SCREEN_SIZE];
	struct gbcc_platform platform;
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
		struct shader shaders[4];
	} gl;
	struct fps_counter fps;
	struct {
		char text[MSG_BUF_SIZE];
		int lines;
		int64_t time_left;
	} msg;
	bool screenshot;
	bool raw_screenshot;
	bool fractional_scaling;
	bool vram_display;
	bool initialised;
};

void gbcc_window_initialise(struct gbcc *gbc);
void gbcc_window_deinitialise(struct gbcc *gbc);
void gbcc_window_update(struct gbcc *gbc);
void gbcc_window_clear();
void gbcc_window_show_message(struct gbcc *gbc, const char *msg, int seconds, bool pad);
void gbcc_window_use_shader(struct gbcc *gbc, const char *name);
void gbcc_load_shader(GLuint shader, const char *filename);
GLuint gbcc_create_shader_program(const char *vert, const char *frag);

#endif /* GBCC_WINDOW_H */
