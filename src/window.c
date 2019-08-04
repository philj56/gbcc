#include "gbcc.h"
#include "colour.h"
#include "constants.h"
#include "debug.h"
#include "fontmap.h"
#include "input.h"
#include "memory.h"
#include "screenshot.h"
#include "time.h"
#include "window.h"
#include <epoxy/gl.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

#ifndef TILESET_PATH
#define TILESET_PATH "tileset.png"
#endif

#ifndef SHADER_PATH
#define SHADER_PATH "shaders/"
#endif

static void render_text(struct gbcc_window *win, const char *text, uint8_t x, uint8_t y);
static void render_character(struct gbcc_window *win, char c, uint8_t x, uint8_t y);
static void update_text(struct gbcc_window *win);

void gbcc_window_initialise(struct gbcc_window *win, struct gbc *gbc)
{
	if (win->initialised) {
		gbcc_log_error("Window already initialised!\n");
		return;
	}
	win->gbc = gbc;
	clock_gettime(CLOCK_REALTIME, &win->fps.last_time);
	gbcc_fontmap_load(&win->font, TILESET_PATH);

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
		gbcc_log_error("Failed to initialize SDL: %s\n", SDL_GetError());
	}

	/* Main OpenGL settings */
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);


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

	win->gl.context = SDL_GL_CreateContext(win->window);

	/* Compile and link the shader programs */
	win->gl.base_shader = gbcc_create_shader_program(
			SHADER_PATH "flipped.vert",
			SHADER_PATH "nothing.frag"
			);

	win->gl.shaders[0].name = "Colour Correct";
	win->gl.shaders[0].program = gbcc_create_shader_program(
			SHADER_PATH "vert.vert",
			SHADER_PATH "colour-correct.frag"
			);

	win->gl.shaders[1].name = "Subpixel";
	win->gl.shaders[1].program = gbcc_create_shader_program(
			SHADER_PATH "vert.vert",
			SHADER_PATH "subpixel.frag"
			);

	win->gl.shaders[2].name = "Nothing";
	win->gl.shaders[2].program = gbcc_create_shader_program(
			SHADER_PATH "vert.vert",
			SHADER_PATH "nothing.frag"
			);

	win->gl.shaders[3].name = "Dot Matrix";
	win->gl.shaders[3].program = gbcc_create_shader_program(
			SHADER_PATH "vert.vert",
			SHADER_PATH "dotmatrix.frag"
			);

	/* Buffer texture param for colour correction LUT */
	GLuint tbo;
	glGenBuffers(1, &tbo);
	glBindBuffer(GL_TEXTURE_BUFFER, tbo);
	uint32_t *lut_data = malloc(32 * 32 * 32 * sizeof(GLuint));
	gbcc_fill_lut(lut_data);
	glBufferData(GL_TEXTURE_BUFFER, 32 * 32 * 32 * sizeof(GLuint), lut_data, GL_STATIC_DRAW);
	free(lut_data);

	glActiveTexture(GL_TEXTURE0+1);
	GLuint lut;
	glGenTextures(1, &lut);
	glBindTexture(GL_TEXTURE_BUFFER, lut);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8, tbo);

	GLint loc = glGetUniformLocation(win->gl.shaders[0].program, "lut");
	glUseProgram(win->gl.shaders[0].program);
	glUniform1i(loc, 1);
	glActiveTexture(GL_TEXTURE0);


	/* Create a vertex buffer for a quad filling the screen */
	float vertices[] = {
		//  Position      Texcoords
		-1.0f,  1.0f, 0.0f, 0.0f, // Top-left
		1.0f,  1.0f,  1.0f, 0.0f, // Top-right
		1.0f, -1.0f,  1.0f, 1.0f, // Bottom-right
		-1.0f, -1.0f, 0.0f, 1.0f  // Bottom-left
	};

	glGenBuffers(1, &win->gl.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, win->gl.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	/* Create a vertex array and enable vertex attributes for the shaders.
	 * TODO: why do we only need to do this once? */
	glGenVertexArrays(1, &win->gl.vao);
	glBindVertexArray(win->gl.vao);

	GLint posAttrib = glGetAttribLocation(win->gl.shaders[0].program, "position");
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
	glEnableVertexAttribArray(posAttrib);

	GLint texAttrib = glGetAttribLocation(win->gl.shaders[0].program, "texcoord");
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void *)(2*sizeof(float)));
	glEnableVertexAttribArray(texAttrib);

	glBindVertexArray(0);

	/* Create the element buffer object that will actually be drawn via
	 * glDrawElements(). */
	GLuint elements[] = {
		0, 1, 2,
		2, 3, 0
	};

	glGenBuffers(1, &win->gl.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, win->gl.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Framebuffer for post-processing */
	glGenFramebuffers(1, &win->gl.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, win->gl.fbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	glGenTextures(1, &win->gl.fbo_texture);
	glBindTexture(GL_TEXTURE_2D, win->gl.fbo_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GBC_SCREEN_WIDTH, GBC_SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, win->gl.fbo_texture, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/* We don't care about the depth and stencil, so just use a
	 * renderbuffer */
	glGenRenderbuffers(1, &win->gl.rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, win->gl.rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, GBC_SCREEN_WIDTH, GBC_SCREEN_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, win->gl.rbo);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		gbcc_log_error("Framebuffer is not complete!\n");
		exit(EXIT_FAILURE);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/* This is the texture we're going to render the raw screen output to
	 * before post-processing */
	glGenTextures(1, &win->gl.texture);
	glBindTexture(GL_TEXTURE_2D, win->gl.texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GBC_SCREEN_WIDTH, GBC_SCREEN_HEIGHT, 0, GL_RGBA,
			GL_UNSIGNED_INT_8_8_8_8, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	/* Bind the actual bits we'll be using to render */
	glBindVertexArray(win->gl.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, win->gl.ebo);
	
	win->initialised = true;
}

void gbcc_window_destroy(struct gbcc_window *win)
{
	if (!win->initialised) {
		gbcc_log_error("Window not initialised!\n");
		return;
	}
	if (win->vram.initialised) {
		gbcc_vram_window_destroy(&win->vram);
	}
	SDL_GL_MakeCurrent(win->window, win->gl.context);
	gbcc_fontmap_destroy(&win->font);
	glDeleteBuffers(1, &win->gl.vbo);
	glDeleteVertexArrays(1, &win->gl.vao);
	glDeleteBuffers(1, &win->gl.ebo);
	glDeleteFramebuffers(1, &win->gl.fbo);
	glDeleteTextures(1, &win->gl.fbo_texture);
	glDeleteRenderbuffers(1, &win->gl.rbo);
	glDeleteTextures(1, &win->gl.texture);
	for (size_t i = 0; i < sizeof(win->gl.shaders) / sizeof(*win->gl.shaders); i++) {
		glDeleteProgram(win->gl.shaders[i].program);
	}
	glDeleteProgram(win->gl.base_shader);
	SDL_GL_DeleteContext(win->gl.context);
	SDL_DestroyWindow(win->window);
	win->initialised = false;
}

void gbcc_window_update(struct gbcc_window *win)
{
	if (win->vram_display) {
		if (!win->vram.initialised) {
			gbcc_vram_window_initialise(&win->vram, win->gbc);

			/* 
			 * Workaround to allow input on vram window.
			 * The VRAM window immediately grabs input focus,
			 * causing an SDL_KEYDOWN to fire, and closing the
			 * window instantly :). To stop that, we make sure that
			 * the main window has focus, and then discard any
			 * SDL_KEYDOWN events.
			 */
			SDL_RaiseWindow(win->window);
			SDL_PumpEvents();
			SDL_FlushEvent(SDL_KEYDOWN);
		}
		gbcc_vram_window_update(&win->vram);
	} else if (win->vram.initialised) {
		gbcc_vram_window_destroy(&win->vram);
	}

	SDL_GL_MakeCurrent(win->window, win->gl.context);
	uint32_t *screen = win->gbc->ppu.screen.sdl;
	bool screenshot = win->screenshot || win->raw_screenshot;

	memcpy(win->buffer, screen, GBC_SCREEN_SIZE * sizeof(*screen));

	update_text(win);
	if (win->fps.show && !screenshot) {
		char fps_text[16];
		snprintf(fps_text, 16, " FPS: %.0f ", round(win->fps.fps));
		render_text(win, fps_text, 0, 0);
	}
	if (win->msg.time_left > 0 && !screenshot) {
		render_text(win, win->msg.text, 0, GBC_SCREEN_HEIGHT - win->msg.lines * win->font.tile_height);
	}

	/* First pass - render the gbc screen to the framebuffer */
	glBindFramebuffer(GL_FRAMEBUFFER, win->gl.fbo);
	glViewport(0, 0, GBC_SCREEN_WIDTH, GBC_SCREEN_HEIGHT);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, win->gl.texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GBC_SCREEN_WIDTH, GBC_SCREEN_HEIGHT, GL_RGBA,
			GL_UNSIGNED_INT_8_8_8_8, (GLvoid *)win->buffer);
	glUseProgram(win->gl.base_shader);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	/* Second pass - render the framebuffer to the screen */
	int width;
	int height;
	SDL_GL_GetDrawableSize(win->window, &width, &height);
	double scale;
	if (win->fractional_scaling) {
		scale = min((double)width / GBC_SCREEN_WIDTH, (double)height / GBC_SCREEN_HEIGHT);
	} else {
		scale = min(width / GBC_SCREEN_WIDTH, height / GBC_SCREEN_HEIGHT);
	}
	win->width = scale * GBC_SCREEN_WIDTH;
	win->height = scale * GBC_SCREEN_HEIGHT;
	win->x = (width - win->width) / 2;
	win->y = (height - win->height) / 2;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(win->x, win->y, win->width, win->height);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, win->gl.fbo_texture);
	glUseProgram(win->gl.shaders[win->gl.cur_shader].program);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	if (screenshot) {
		gbcc_screenshot(win);
	}

	SDL_GL_SwapWindow(win->window);
}

void gbcc_load_shader(GLuint shader, const char *filename)
{
	FILE *fp = fopen(filename, "rbe");
	if (!fp) {
		gbcc_log_error("Failed to load shader %s.\n", filename);
		exit(EXIT_FAILURE);
	}
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	GLchar *source = malloc(size);
	rewind(fp);
	fread(source, 1, size, fp);
	fclose(fp);
	glShaderSource(shader, 1, (const GLchar *const *)&source, &size);
	free(source);

	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		gbcc_log_error("Failed to compile shader %s!\n", filename);

		GLint info_length = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_length);
		if (info_length > 1) {
			char *log = malloc(info_length * sizeof(*log));
			glGetShaderInfoLog(shader, info_length, NULL, log);
			gbcc_log_append_error("%s\n", log);
			free(log);
		}
		exit(EXIT_FAILURE);
	}
}

GLuint gbcc_create_shader_program(const char *vert, const char *frag)
{
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	gbcc_load_shader(vertex_shader, vert);

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	gbcc_load_shader(fragment_shader, frag);

	GLuint shader = glCreateProgram();
	glAttachShader(shader, vertex_shader);
	glAttachShader(shader, fragment_shader);
	glBindFragDataLocation(shader, 0, "out_colour");
	glLinkProgram(shader);
	return shader;
}

void gbcc_window_use_shader(struct gbcc_window *win, const char *name)
{
	int num_shaders = sizeof(win->gl.shaders) / sizeof(win->gl.shaders[0]);
	int s;
	for (s = 0; s < num_shaders; s++) {
		if (strcasecmp(name, win->gl.shaders[s].name) == 0) {
			break;
		}
	}
	if (s >= num_shaders) {
		gbcc_log_error("Invalid shader \"%s\"\n", name);
	} else {
		win->gl.cur_shader = s;
	}
}

void gbcc_window_show_message(struct gbcc_window *win, const char *msg, int seconds, bool pad)
{
	if (pad) {
		snprintf(win->msg.text, MSG_BUF_SIZE, " %s ", msg);
	} else {
		strncpy(win->msg.text, msg, MSG_BUF_SIZE);
	}
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
			uint32_t dst_px = (j + y) * GBC_SCREEN_WIDTH + (i + x);
			if (win->font.bitmap[src_px] > 0) {
				win->buffer[dst_px] = 0xFFFFFFFFu;
				continue;
			}
			uint8_t r = (win->buffer[dst_px] >> 24u) & 0xFFu;
			uint8_t g = (win->buffer[dst_px] >> 16u) & 0xFFu;
			uint8_t b = (win->buffer[dst_px] >> 8u) & 0xFFu;
			uint32_t res = 0;
			r = (uint8_t)round(r / 4.0);
			g = (uint8_t)round(g / 4.0);
			b = (uint8_t)round(b / 4.0);
			res |= (uint32_t)(r << 24u);
			res |= (uint32_t)(g << 16u);
			res |= (uint32_t)(b << 8u);
			res |= 0xFFu;
			win->buffer[dst_px] = res;
		}
	}
}

void update_text(struct gbcc_window *win)
{
	struct fps_counter *fps = &win->fps;
	struct timespec cur_time;
	clock_gettime(CLOCK_REALTIME, &cur_time);
	double dt = gbcc_time_diff(&cur_time, &fps->last_time);

	/* Update FPS counter */
	fps->last_time = cur_time;
	float df = win->gbc->ppu.frame - fps->last_frame;
	uint8_t ly = gbcc_memory_read(win->gbc, LY, false);
	uint8_t tmp = ly;
	if (ly < fps->last_ly) {
		df -= 1;
		ly += (154 - fps->last_ly);
	} else {
		ly -= fps->last_ly;
	}
	df += ly / 154.0;
	fps->last_frame = win->gbc->ppu.frame;
	fps->last_ly = tmp;
	fps->previous[fps->idx] = df / (dt / 1e9);
	fps->idx++;
	fps->idx %= 5;
	float avg = 0;
	for (int i = 0; i < 5; i++) {
		avg += fps->previous[i];
	}
	fps->fps = avg / 5;

	/* Update message timer */
	if (win->msg.time_left > 0) {
		win->msg.time_left -= (int64_t)dt;
	}
}
