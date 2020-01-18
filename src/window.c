#include "gbcc.h"
#include "colour.h"
#include "constants.h"
#include "debug.h"
#include "fontmap.h"
#include "memory.h"
#include "nelem.h"
#include "screenshot.h"
#include "time_diff.h"
#include "window.h"
#ifdef __ANDROID__
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#else
#include <epoxy/gl.h>
#endif
#include <math.h>
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
static void render_box(struct gbcc_window *win, uint8_t x, uint8_t y);
static void update_text(struct gbcc *gbc);

void gbcc_window_initialise(struct gbcc *gbc)
{
	struct gbcc_window *win = &gbc->window;

	GLint read_framebuffer = 0;
	GLint draw_framebuffer = 0;
	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_framebuffer);
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_framebuffer);

	clock_gettime(CLOCK_REALTIME, &win->fps.last_time);
	gbcc_fontmap_load(&win->font, TILESET_PATH);

	/* Compile and link the shader programs */
	win->gl.base_shader = gbcc_create_shader_program(
			SHADER_PATH "flipped.vert",
			SHADER_PATH "frameblend.frag"
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

	GLint posAttrib = glGetAttribLocation(win->gl.base_shader, "position");
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
	glEnableVertexAttribArray(posAttrib);

	GLint texAttrib = glGetAttribLocation(win->gl.base_shader, "texcoord");
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
	glGenTextures(1, &win->gl.fbo_texture);
	glBindTexture(GL_TEXTURE_2D, win->gl.fbo_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GBC_SCREEN_WIDTH, GBC_SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &win->gl.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, win->gl.fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, win->gl.fbo_texture, 0);

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
	glBindFramebuffer(GL_READ_FRAMEBUFFER, read_framebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_framebuffer);

	/* This is the texture we're going to render the raw screen output to
	 * before post-processing */
	glGenTextures(1, &win->gl.texture);
	glBindTexture(GL_TEXTURE_2D, win->gl.texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GBC_SCREEN_WIDTH * 2, GBC_SCREEN_HEIGHT, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	/* This is the 3D texture we use as a lookup-table for colour correction */
	glGenTextures(1, &win->gl.lut_texture);

	uint8_t (*lut_data)[8][8][4] = malloc(8 * 8 * 8 * 4);
	gbcc_fill_lut(lut_data);

	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_3D, win->gl.lut_texture);
	glUseProgram(win->gl.shaders[0].program);

	GLint loc = glGetUniformLocation(win->gl.shaders[0].program, "lut");
	glUniform1i(loc, 1);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 8, 8, 8, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, (GLvoid *)lut_data);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glActiveTexture(GL_TEXTURE0);
	free(lut_data);

	/* Bind the actual bits we'll be using to render */
	glBindVertexArray(win->gl.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, win->gl.ebo);

	win->initialised = true;
}

void gbcc_window_deinitialise(struct gbcc *gbc)
{
	struct gbcc_window *win = &gbc->window;
	win->initialised = false;

	gbcc_fontmap_destroy(&win->font);
	glDeleteBuffers(1, &win->gl.vbo);
	glDeleteVertexArrays(1, &win->gl.vao);
	glDeleteBuffers(1, &win->gl.ebo);
	glDeleteFramebuffers(1, &win->gl.fbo);
	glDeleteTextures(1, &win->gl.fbo_texture);
	glDeleteRenderbuffers(1, &win->gl.rbo);
	glDeleteTextures(1, &win->gl.texture);
	glDeleteTextures(1, &win->gl.lut_texture);
	for (size_t i = 0; i < N_ELEM(win->gl.shaders); i++) {
		glDeleteProgram(win->gl.shaders[i].program);
	}
	glDeleteProgram(win->gl.base_shader);
}

void gbcc_window_clear()
{
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gbcc_window_update(struct gbcc *gbc)
{
	struct gbcc_window *win = &gbc->window;
	GLint read_framebuffer = 0;
	GLint draw_framebuffer = 0;
	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_framebuffer);
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_framebuffer);

	uint32_t *screen = gbc->core.ppu.screen.sdl;
	bool screenshot = win->screenshot || win->raw_screenshot;

	if (win->frame_blending) {
		memcpy(win->last_buffer, win->buffer, GBC_SCREEN_SIZE * sizeof(*screen));
	}
	memcpy(win->buffer, screen, GBC_SCREEN_SIZE * sizeof(*screen));

	if (gbc->menu.show) {
		uint32_t tw = win->font.tile_width;
		uint32_t th = win->font.tile_height;
		render_text(win, gbc->menu.text, 0, 0);
		render_box(win, 11.5 * tw + 2 + gbc->menu.save_state * tw * 2, th * 2 + 1);
		render_box(win, 11.5 * tw + 2 + gbc->menu.load_state * tw * 2, th * 3 + 1);
	} else {
		update_text(gbc);
		if (win->fps.show && !screenshot) {
			char fps_text[16];
			snprintf(fps_text, 16, " FPS: %.0f ", win->fps.fps);
			render_text(win, fps_text, 0, 0);
		}
		if (win->msg.time_left > 0 && !screenshot) {
			render_text(win, win->msg.text, 0, GBC_SCREEN_HEIGHT - win->msg.lines * win->font.tile_height);
		}
	}

	for (size_t i = 0; i < N_ELEM(win->buffer); i++) {
		uint32_t tmp = win->buffer[i];
		win->buffer[i] = (tmp & 0xFFu) << 24u
				| (tmp & 0xFF00u) << 8u
				| (tmp & 0xFF0000u) >> 8u
				| (tmp & 0xFF000000u) >> 24u;
	}

	if (!win->frame_blending) {
		memcpy(win->last_buffer, win->buffer, GBC_SCREEN_SIZE * sizeof(*screen));
	}

	/* First pass - render the gbc screen to the framebuffer */
	glBindFramebuffer(GL_FRAMEBUFFER, win->gl.fbo);
	glViewport(0, 0, GBC_SCREEN_WIDTH, GBC_SCREEN_HEIGHT);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, win->gl.texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GBC_SCREEN_WIDTH, GBC_SCREEN_HEIGHT, GL_RGBA,
			GL_UNSIGNED_BYTE, (GLvoid *)win->buffer);
	glTexSubImage2D(GL_TEXTURE_2D, 0, GBC_SCREEN_WIDTH, 0, GBC_SCREEN_WIDTH, GBC_SCREEN_HEIGHT, GL_RGBA,
			GL_UNSIGNED_BYTE, (GLvoid *)win->last_buffer);
	glUseProgram(win->gl.base_shader);
	glUniform1i( glGetUniformLocation( win->gl.base_shader, "tex"), 0);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	/* Second pass - render the framebuffer to the screen */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, read_framebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_framebuffer);

	if (win->fractional_scaling) {
		win->scale = min((double)win->width / GBC_SCREEN_WIDTH, (double)win->height / GBC_SCREEN_HEIGHT);
	} else {
		win->scale = min(win->width / GBC_SCREEN_WIDTH, win->height / GBC_SCREEN_HEIGHT);
	}
	int width = win->scale * GBC_SCREEN_WIDTH;
	int height = win->scale * GBC_SCREEN_HEIGHT;
	win->x = (win->width - width) / 2;
	win->y = (win->height - height) / 2;
	glViewport(win->x, win->y, width, height);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, win->gl.fbo_texture);
	glUseProgram(win->gl.shaders[win->gl.cur_shader].program);
	glUniform1i( glGetUniformLocation( win->gl.shaders[win->gl.cur_shader].program, "tex"), 0);
	glUniform1i( glGetUniformLocation( win->gl.shaders[win->gl.cur_shader].program, "lut"), 1);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	if (screenshot) {
		gbcc_screenshot(gbc);
	}
}

void gbcc_load_shader(GLuint shader, const char *filename)
{
	FILE *fp = fopen(filename, "rb");
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
#ifndef __ANDROID__
	glBindFragDataLocation(shader, 0, "out_colour");
#endif
	glLinkProgram(shader);
	return shader;
}

void gbcc_window_use_shader(struct gbcc *gbc, const char *name)
{
	struct gbcc_window *win = &gbc->window;
	int num_shaders = N_ELEM(win->gl.shaders);
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

void gbcc_window_show_message(struct gbcc *gbc, const char *msg, int seconds, bool pad)
{
	struct gbcc_window *win = &gbc->window;
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
			r /= 4;
			g /= 4;
			b /= 4;
			res |= (uint32_t)(r << 24u);
			res |= (uint32_t)(g << 16u);
			res |= (uint32_t)(b << 8u);
			res |= 0xFFu;
			win->buffer[dst_px] = res;
		}
	}
}

void render_box(struct gbcc_window *win, uint8_t x, uint8_t y)
{
	uint8_t width = win->font.tile_width * 2 - 3;
	uint8_t height = win->font.tile_height - 3;
	for (int i = x; i <= x + width; i++) {
		win->buffer[y * GBC_SCREEN_WIDTH + i] = 0xFFFFFFFFu;
		win->buffer[(y + height) * GBC_SCREEN_WIDTH + i] = 0xFFFFFFFFu;
	}
	for (int i = y; i <= y + height; i++) {
		win->buffer[i * GBC_SCREEN_WIDTH + x] = 0xFFFFFFFFu;
		win->buffer[i * GBC_SCREEN_WIDTH + x + width] = 0xFFFFFFFFu;
	}
}

void update_text(struct gbcc *gbc)
{
	struct gbcc_window *win = &gbc->window;
	struct fps_counter *fps = &win->fps;
	struct timespec cur_time;
	clock_gettime(CLOCK_REALTIME, &cur_time);
	double dt = gbcc_time_diff(&cur_time, &fps->last_time);

	/* Update FPS counter */
	fps->last_time = cur_time;
	float df = gbc->core.ppu.frame - fps->last_frame;
	uint8_t ly = gbcc_memory_read(&gbc->core, LY, true);
	uint8_t tmp = ly;
	if (ly < fps->last_ly) {
		df -= 1;
		ly += (154 - fps->last_ly);
	} else {
		ly -= fps->last_ly;
	}
	df += ly / 154.0;
	fps->last_frame = gbc->core.ppu.frame;
	fps->last_ly = tmp;
	fps->previous[fps->idx] = df / (dt / 1e9);
	fps->idx++;
	fps->idx %= N_ELEM(fps->previous);
	float avg = 0;
	for (size_t i = 0; i < N_ELEM(fps->previous); i++) {
		avg += fps->previous[i];
	}
	fps->fps = avg / N_ELEM(fps->previous);

	/* Update message timer */
	if (win->msg.time_left > 0) {
		win->msg.time_left -= (int64_t)dt;
	}
}
