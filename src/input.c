#include "gbcc.h"
#include "debug.h"
#include "input.h"
#include "memory.h"
#include "save.h"
#include "window.h"
#include <SDL2/SDL.h>

static const SDL_Scancode keymap[25] = {
	SDL_SCANCODE_Z,		/* A */
	SDL_SCANCODE_X, 	/* B */
	SDL_SCANCODE_RETURN,	/* Start */
	SDL_SCANCODE_SPACE,	/* Select */
	SDL_SCANCODE_UP,	/* DPAD up */
	SDL_SCANCODE_DOWN,	/* DPAD down */
	SDL_SCANCODE_LEFT,	/* DPAD left */
	SDL_SCANCODE_RIGHT,	/* DPAD right */
	SDL_SCANCODE_RSHIFT, 	/* Turbo */
	SDL_SCANCODE_S, 	/* Screenshot */
	SDL_SCANCODE_P, 	/* Pause */
	SDL_SCANCODE_F, 	/* FPS Counter */
	SDL_SCANCODE_V, 	/* Switch shader */
	SDL_SCANCODE_1, 	/* Toggle background */
	SDL_SCANCODE_2, 	/* Toggle window */
	SDL_SCANCODE_3, 	/* Toggle sprites */
	SDL_SCANCODE_F1,	/* State n */
	SDL_SCANCODE_F2,
	SDL_SCANCODE_F3,
	SDL_SCANCODE_F4,
	SDL_SCANCODE_F5,
	SDL_SCANCODE_F6,
	SDL_SCANCODE_F7,
	SDL_SCANCODE_F8,
	SDL_SCANCODE_F9
};

// Returns key that changed, or -1 for a non-emulated key
static int process_input(struct gbc *gbc, const SDL_Event *e);

void gbcc_input_process_all(struct gbcc_window *win)
{
	struct gbc *gbc = win->gbc;
	SDL_Event e;
	while (SDL_PollEvent(&e) != 0) {
		int key = process_input(gbc, &e);
		const uint8_t *state = SDL_GetKeyboardState(NULL);
		bool val;
		if (e.type == SDL_KEYDOWN) {
			val = true;
		//	gbc->halt.set = false;
		//	gbc->stop = false;
		} else {
			val = false;
		}
		switch(key) {
			case 0:
				gbc->keys.a = val;
				gbcc_memory_set_bit(gbc, IF, 4, true);
				break;
			case 1:
				gbc->keys.b = val;
				gbcc_memory_set_bit(gbc, IF, 4, true);
				break;
			case 2:
				gbc->keys.start = val;
				gbcc_memory_set_bit(gbc, IF, 4, true);
				break;
			case 3:
				gbc->keys.select = val;
				gbcc_memory_set_bit(gbc, IF, 4, true);
				break;
			case 4:
				gbc->keys.dpad.up = val;
				gbcc_memory_set_bit(gbc, IF, 4, true);
				break;
			case 5:
				gbc->keys.dpad.down = val;
				gbcc_memory_set_bit(gbc, IF, 4, true);
				break;
			case 6:
				gbc->keys.dpad.left = val;
				gbcc_memory_set_bit(gbc, IF, 4, true);
				break;
			case 7:
				gbc->keys.dpad.right = val;
				gbcc_memory_set_bit(gbc, IF, 4, true);
				break;
			case 8:
				gbc->keys.turbo ^= val;
				/* TODO: This shouldn't really be done here */
				gbc->apu.start_time = gbc->apu.cur_time;
				gbc->apu.sample = 0;
				break;
			case 9:
				if (state[SDL_SCANCODE_LSHIFT]) {
					win->raw_screenshot ^= val;
				} else {
					win->screenshot ^= val;
				}
				break;
			case 10:
				gbc->pause ^= val;
				break;
			case 11:
				win->fps.show ^= val;
				break;
			case 12:
				win->gl.cur_shader += val;
				win->gl.cur_shader %= sizeof(win->gl.shaders) / sizeof(win->gl.shaders[0]);
				gbcc_window_show_message(win, win->gl.shaders[win->gl.cur_shader].name, 1, true);
				break;
			case 13:
				gbc->hide_background ^= val;
				if (!val) {
					break;
				}
				if (gbc->hide_background) {
					gbcc_window_show_message(win, "Background disabled", 1, true);
				} else {
					gbcc_window_show_message(win, "Background enabled", 1, true);
				}
				break;
			case 14:
				gbc->hide_window ^= val;
				if (!val) {
					break;
				}
				if (gbc->hide_window) {
					gbcc_window_show_message(win, "Window disabled", 1, true);
				} else {
					gbcc_window_show_message(win, "Window enabled", 1, true);
				}
				break;
			case 15:
				gbc->hide_sprites ^= val;
				if (!val) {
					break;
				}
				if (gbc->hide_sprites) {
					gbcc_window_show_message(win, "Sprites disabled", 1, true);
				} else {
					gbcc_window_show_message(win, "Sprites enabled", 1, true);
				}
				break;
			case 16:
			case 17:
			case 18:
			case 19:
			case 20:
			case 21:
			case 22:
			case 23:
			case 24:
				if (!val) {
					break;
				}
				if (state[SDL_SCANCODE_LSHIFT]) {
					gbc->save_state = (int8_t)(key - 15);
				} else {
					gbc->load_state = (int8_t)(key - 15);
				}
				break;
			default:
				break;
		}
	}
}

int process_input(struct gbc *gbc, const SDL_Event *e)
{
	if (e->type == SDL_QUIT) {
		gbc->quit = true;
	} else if (e->type == SDL_WINDOWEVENT) {
		if (e->window.event == SDL_WINDOWEVENT_CLOSE) {
			SDL_Window *win = SDL_GetWindowFromID(e->window.windowID);
			if (win) {
				SDL_DestroyWindow(win);
				/* TODO: Should quit if main window is closed */
			} else {
				gbcc_log_error("Couldn't close window: %s\n", SDL_GetError());
			}
		}
	} else if (e->type == SDL_KEYDOWN || e->type == SDL_KEYUP) {
		for (size_t i = 0; i < sizeof(keymap) / sizeof(keymap[0]); i++) {
			if (e->key.keysym.scancode == keymap[i]) {
				return (int)i;
			}
		}
	}
	return -1;
}
