#ifndef GBCC_VRAM_WINDOW_H
#define GBCC_VRAM_WINDOW_H

#include "gbcc.h"
#include <epoxy/gl.h>
#include <SDL2/SDL.h>
#include <stdint.h>

#define VRAM_WINDOW_WIDTH_TILES (24)
#define VRAM_WINDOW_HEIGHT_TILES (32)

#define VRAM_WINDOW_WIDTH (8*VRAM_WINDOW_WIDTH_TILES)
#define VRAM_WINDOW_HEIGHT (8*VRAM_WINDOW_HEIGHT_TILES)
#define VRAM_WINDOW_SIZE (VRAM_WINDOW_WIDTH * VRAM_WINDOW_HEIGHT)

struct gbcc_vram_window {
	struct gbc *gbc;
	SDL_Window *window;
	uint32_t width;
	uint32_t height;
	uint32_t x;
	uint32_t y;
	uint32_t buffer[VRAM_WINDOW_SIZE];
	struct {
		SDL_GLContext context;
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

void gbcc_vram_window_initialise(struct gbcc_vram_window *win, struct gbc *gbc);
void gbcc_vram_window_destroy(struct gbcc_vram_window *win);
void gbcc_vram_window_update(struct gbcc_vram_window *win);

#endif /* GBCC_VRAM_WINDOW_H */
