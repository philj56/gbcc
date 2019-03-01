#include "gbcc.h"
#include "gbcc_colour.h"
#include "gbcc_constants.h"
#include "gbcc_debug.h"
#include "gbcc_fontmap.h"
#include "gbcc_input.h"
#include "gbcc_memory.h"
#include "gbcc_screenshot.h"
#include "gbcc_time.h"
#include "gbcc_window.h"
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

#ifndef TILESET_PATH
#define TILESET_PATH "/usr/share/gbcc/tileset.png"
#endif

static void render_text(struct gbcc_window *win, const char *text, uint8_t x, uint8_t y);
static void render_character(struct gbcc_window *win, char c, uint8_t x, uint8_t y);
static void update_text(struct gbcc_window *win);
static uint32_t subpixel_lut[32][32][32][6];

static void load_shader(GLuint shader, const char *filename)
{
	FILE *fp = fopen(filename, "rbe");
	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	GLchar *source = malloc(size);
	rewind(fp);
	fread(source, 1, size, fp);
	fclose(fp);
	glShaderSource(shader, 1, &source, &size);
	free(source);
}

static void fill_lookup()
{
	const uint32_t red = 0xFF7145;
	const uint32_t green = 0xC1D650;
	const uint32_t blue = 0x3BCEFF;
	static bool done = false;
	if (done) {
		return;
	}
	for (uint32_t r = 0; r < 32; r++) {
		for (uint32_t g = 0; g < 32; g++) {
			for (uint32_t b = 0; b < 32; b++) {
				uint32_t r2 = gbcc_add_colours(0, red, r / 32.0);
				uint32_t g2 = gbcc_add_colours(0, green, g / 32.0);
				uint32_t b2 = gbcc_add_colours(0, blue, b / 32.0);
				subpixel_lut[r][g][b][0] = gbcc_add_colours(r2, 0, 0.7);
				subpixel_lut[r][g][b][1] = gbcc_add_colours(r2, g2, 0.7);
				subpixel_lut[r][g][b][2] = gbcc_add_colours(g2, r2, 0.7);
				subpixel_lut[r][g][b][3] = gbcc_add_colours(g2, b2, 0.7);
				subpixel_lut[r][g][b][4] = gbcc_add_colours(b2, g2, 0.7);
				subpixel_lut[r][g][b][5] = gbcc_add_colours(b2, 0, 0.7);
			}
		}
	}
	done = true;
}

void gbcc_window_initialise(struct gbcc_window *win, struct gbc *gbc, enum scaling_type scaling)
{
	win->scaling.type = scaling;
	switch (scaling) {
		case SCALING_NONE:
			win->scaling.factor = 1;
			break;
		case SCALING_SUBPIXEL:
			win->scaling.factor = 7;
			break;
		default:
			gbcc_log_error("Invalid scaling type\n");
			exit(EXIT_FAILURE);
	}
	win->gbc = gbc;
	clock_gettime(CLOCK_REALTIME, &win->fps_counter.last_time);
	gbcc_fontmap_load(&win->font, TILESET_PATH);

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
		gbcc_log_error("Failed to initialize SDL: %s\n", SDL_GetError());
	}
	if (scaling == SCALING_SUBPIXEL){
		SDL_DisplayMode dm;
		SDL_GetDesktopDisplayMode(0, &dm);
		if (dm.w < 3 * GBC_SCREEN_WIDTH || dm.h < 3 * GBC_SCREEN_HEIGHT) {
			gbcc_log_warning("Minimum recommended screen size for subpixel scaling is %dx%d\n",
					3 * GBC_SCREEN_WIDTH, 3 * GBC_SCREEN_HEIGHT);
		}
	}

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
	SDL_GL_CreateContext(win->window);


	glewExperimental = GL_TRUE;
	glewInit();

	float vertices[] = {
		//  Position      Texcoords
		-1.0f,  1.0f, 0.0f, 0.0f, // Top-left
		1.0f,  1.0f,  1.0f, 0.0f, // Top-right
		1.0f, -1.0f,  1.0f, 1.0f, // Bottom-right
		-1.0f, -1.0f, 0.0f, 1.0f  // Bottom-left
	};

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	load_shader(vertexShader, "vert.vert");
	glCompileShader(vertexShader);

	GLint status;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);

	if (status != GL_TRUE) {
		gbcc_log_error("Could not compile vertex shader!\n");
		exit(EXIT_FAILURE);
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	load_shader(fragmentShader, "default.frag");
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);

	if (status != GL_TRUE) {
		gbcc_log_error("Could not compile fragment shader!\n");
		exit(EXIT_FAILURE);
	}

	win->gl.shader_program = glCreateProgram();
	glAttachShader(win->gl.shader_program, vertexShader);
	glAttachShader(win->gl.shader_program, fragmentShader);
	glBindFragDataLocation(win->gl.shader_program, 0, "outColor");
	glLinkProgram(win->gl.shader_program);
	glUseProgram(win->gl.shader_program);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint elements[] = {
		0, 1, 2,
		2, 3, 0
	};

	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);


	GLint posAttrib = glGetAttribLocation(win->gl.shader_program, "position");
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
	glEnableVertexAttribArray(posAttrib);

	GLint texAttrib = glGetAttribLocation(win->gl.shader_program, "texcoord");
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void *)(2*sizeof(float)));
	glEnableVertexAttribArray(texAttrib);

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	fill_lookup();
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GBC_SCREEN_WIDTH, GBC_SCREEN_HEIGHT, 0, GL_RGBA,
			GL_UNSIGNED_INT_8_8_8_8, (GLvoid *)win->gbc->ppu.screen.sdl);
}

void gbcc_window_destroy(struct gbcc_window *win)
{
	gbcc_fontmap_destroy(&win->font);
	SDL_DestroyWindow(win->window);
}

void gbcc_window_update(struct gbcc_window *win)
{
	/* TODO: This resize logic should only be done on window resize */
	int width;
	int height;
	SDL_GL_GetDrawableSize(win->window, &width, &height);
	int scale = min(width / GBC_SCREEN_WIDTH, height / GBC_SCREEN_HEIGHT);
	int scaled_width = scale * GBC_SCREEN_WIDTH;
	int scaled_height = scale * GBC_SCREEN_HEIGHT;
	glViewport((width - scaled_width) / 2, (height - scaled_height) / 2, scaled_width, scaled_height);
	uint32_t *screen = win->gbc->ppu.screen.sdl;
	update_text(win);
	if (win->fps_counter.show) {
		char fps_text[16];
		snprintf(fps_text, 16, " FPS: %.0f ", win->fps_counter.fps);
		render_text(win, fps_text, 0, 0);
	}
	if (win->msg.time_left > 0) {
		render_text(win, win->msg.text, 0, GBC_SCREEN_HEIGHT - win->msg.lines * win->font.tile_height);
	}
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GBC_SCREEN_WIDTH, GBC_SCREEN_HEIGHT, GL_RGBA,
			GL_UNSIGNED_INT_8_8_8_8, (GLvoid *)screen);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	SDL_GL_SwapWindow(win->window);
}

void gbcc_window_show_message(struct gbcc_window *win, const char *msg, int seconds)
{
	strncpy(win->msg.text, msg, MSG_BUF_SIZE);
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
			for (uint32_t n = 0; n < win->scaling.factor; n++) {
				for (uint32_t m = 0; m < win->scaling.factor; m++) {
					uint32_t dst_px = (n + (j + y) * win->scaling.factor) * win->scaling.factor * GBC_SCREEN_WIDTH + m + (i + x) * win->scaling.factor;
					if (win->font.bitmap[src_px] > 0) {
						win->gbc->ppu.screen.sdl[dst_px] = 0xFFFFFF00u;
					} else {
						uint8_t r = (win->gbc->ppu.screen.sdl[dst_px] >> 24u) & 0xFFu;
						uint8_t g = (win->gbc->ppu.screen.sdl[dst_px] >> 16u) & 0xFFu;
						uint8_t b = (win->gbc->ppu.screen.sdl[dst_px] >> 8u) & 0xFFu;
						uint32_t res = 0;
						r = (uint8_t)round(r / 4.0);
						g = (uint8_t)round(g / 4.0);
						b = (uint8_t)round(b / 4.0);
						res |= (uint32_t)(r << 24u);
						res |= (uint32_t)(g << 16u);
						res |= (uint32_t)(b << 8u);
						win->gbc->ppu.screen.sdl[dst_px] = res;
					}
				}
			}
		}
	}
}

void update_text(struct gbcc_window *win)
{
	struct timespec cur_time;
	clock_gettime(CLOCK_REALTIME, &cur_time);
	double dt = gbcc_time_diff(&cur_time, &win->fps_counter.last_time);

	/* Update FPS counter */
	win->fps_counter.last_time = cur_time;
	float df = win->gbc->ppu.frame - win->fps_counter.last_frame;
	uint8_t ly = gbcc_memory_read(win->gbc, LY, false);
	if (ly < win->fps_counter.last_ly) {
		df -= 1;
		ly += (154 - win->fps_counter.last_ly);
	} else {
		ly -= win->fps_counter.last_ly;
	}
	df += ly / 154.0;
	win->fps_counter.last_frame = win->gbc->ppu.frame;
	win->fps_counter.last_ly = gbcc_memory_read(win->gbc, LY, false);
	win->fps_counter.fps = df / (dt / 1e9);

	/* Update message timer */
	if (win->msg.time_left > 0) {
		win->msg.time_left -= (int64_t)dt;
	}
}
