#ifndef GBCC_VRAM_WINDOW_H
#define GBCC_VRAM_WINDOW_H

#ifdef GBCC_GTK
#include "gtk/platform.h"
#else
#include "sdl/platform.h"
#endif
#include <epoxy/gl.h>
#include <stdint.h>

#define VRAM_WINDOW_WIDTH_TILES (24)
#define VRAM_WINDOW_HEIGHT_TILES (32)

#define VRAM_WINDOW_WIDTH (8*VRAM_WINDOW_WIDTH_TILES)
#define VRAM_WINDOW_HEIGHT (8*VRAM_WINDOW_HEIGHT_TILES)
#define VRAM_WINDOW_SIZE (VRAM_WINDOW_WIDTH * VRAM_WINDOW_HEIGHT)

struct gbcc;

struct gbcc_vram_window {
	uint32_t x;
	uint32_t y;
	int32_t width;
	int32_t height;
	uint32_t buffer[VRAM_WINDOW_SIZE];
	struct gbcc_platform platform;
	struct {
		GLuint vbo;
		GLuint vao;
		GLuint ebo;
		GLuint fbo;
		GLuint fbo_texture;
		GLuint rbo;
		GLuint texture;
		GLuint shader;
		GLuint flip_shader;
	} gl;
	bool initialised;
};

void gbcc_vram_window_initialise(struct gbcc *gbc);
void gbcc_vram_window_deinitialise(struct gbcc *gbc);
void gbcc_vram_window_update(struct gbcc *gbc);
void gbcc_vram_window_clear();

#endif /* GBCC_VRAM_WINDOW_H */
