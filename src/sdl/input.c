#include "../gbcc.h"
#include "../debug.h"
#include "../input.h"
#include "../nelem.h"
#include "sdl.h"
#include <SDL2/SDL.h>

static const SDL_Scancode keymap[31] = {
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
	SDL_SCANCODE_L, 	/* Toggle link cable loop */
	SDL_SCANCODE_B, 	/* Toggle background playback */
	SDL_SCANCODE_KP_2, 	/* MBC7 accelerometer down */
	SDL_SCANCODE_KP_4, 	/* MBC7 accelerometer left */
	SDL_SCANCODE_KP_6, 	/* MBC7 accelerometer right */
	SDL_SCANCODE_KP_8, 	/* MBC7 accelerometer up */
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
static int process_input(struct gbcc *gbc, const SDL_Event *e);

void gbcc_sdl_process_input(struct gbcc *gbc)
{
	SDL_Event e;
	const uint8_t *state = SDL_GetKeyboardState(NULL);
	while (SDL_PollEvent(&e) != 0) {
		int key = process_input(gbc, &e);
		enum gbcc_key emulator_key;
		bool val;
		if (e.type == SDL_KEYDOWN) {
			val = true;
		//	gbc->halt.set = false;
		//	gbc->cpu.stop = false;
		} else if (e.type == SDL_KEYUP) {
			val = false;
		} else {
			continue;
		}

		switch(key) {
			case 0:
				emulator_key = GBCC_KEY_A;
				break;
			case 1:
				emulator_key = GBCC_KEY_B;
				break;
			case 2:
				emulator_key = GBCC_KEY_START;
				break;
			case 3:
				emulator_key = GBCC_KEY_SELECT;
				break;
			case 4:
				emulator_key = GBCC_KEY_UP;
				break;
			case 5:
				emulator_key = GBCC_KEY_DOWN;
				break;
			case 6:
				emulator_key = GBCC_KEY_LEFT;
				break;
			case 7:
				emulator_key = GBCC_KEY_RIGHT;
				break;
			case 8:
				emulator_key = GBCC_KEY_TURBO;
				break;
			case 9:
				if (state[SDL_SCANCODE_LSHIFT]) {
					emulator_key = GBCC_KEY_RAW_SCREENSHOT;
				} else {
					emulator_key = GBCC_KEY_SCREENSHOT;
				}
				break;
			case 10:
				if (state[SDL_SCANCODE_LSHIFT]) {
					emulator_key = GBCC_KEY_PRINTER;
				} else {
					emulator_key = GBCC_KEY_PAUSE;
				}
				break;
			case 11:
				emulator_key = GBCC_KEY_FPS;
				break;
			case 12:
				if (state[SDL_SCANCODE_LSHIFT]) {
					emulator_key = GBCC_KEY_VRAM;
				} else {
					emulator_key = GBCC_KEY_SHADER;
				}
				break;
			case 13:
				emulator_key = GBCC_KEY_DISPLAY_BACKGROUND;
				break;
			case 14:
				emulator_key = GBCC_KEY_DISPLAY_WINDOW;
				break;
			case 15:
				emulator_key = GBCC_KEY_DISPLAY_SPRITES;
				break;
			case 16:
				emulator_key = GBCC_KEY_LINK_CABLE;
				break;
			case 17:
				emulator_key = GBCC_KEY_BACKGROUND_PLAY;
				break;
			case 18:
				if (state[SDL_SCANCODE_LSHIFT]) {
					emulator_key = GBCC_KEY_ACCELEROMETER_MAX_DOWN;
				} else {
					continue;
				}
				break;
			case 19:
				if (state[SDL_SCANCODE_LSHIFT]) {
					emulator_key = GBCC_KEY_ACCELEROMETER_MAX_LEFT;
				} else {
					continue;
				}
				break;
			case 20:
				if (state[SDL_SCANCODE_LSHIFT]) {
					emulator_key = GBCC_KEY_ACCELEROMETER_MAX_RIGHT;
				} else {
					continue;
				}
				break;
			case 21:
				if (state[SDL_SCANCODE_LSHIFT]) {
					emulator_key = GBCC_KEY_ACCELEROMETER_MAX_UP;
				} else {
					continue;
				}
				break;
			case 22:
			case 23:
			case 24:
			case 25:
			case 26:
			case 27:
			case 28:
			case 29:
			case 30:
				if (state[SDL_SCANCODE_LSHIFT]) {
					emulator_key = GBCC_KEY_SAVE_STATE_1 + (int8_t)(key - 22);
				} else {
					emulator_key = GBCC_KEY_LOAD_STATE_1 + (int8_t)(key - 22);
				}
				break;
			default:
				continue;
		}
		gbcc_input_process_key(gbc, emulator_key, val);
	}

	if (state[SDL_SCANCODE_KP_6]) {
		gbcc_input_process_key(gbc, GBCC_KEY_ACCELEROMETER_RIGHT, true);
		gbcc_input_process_key(gbc, GBCC_KEY_ACCELEROMETER_LEFT, false);
	} else if (state[SDL_SCANCODE_KP_4]) {
		gbcc_input_process_key(gbc, GBCC_KEY_ACCELEROMETER_RIGHT, false);
		gbcc_input_process_key(gbc, GBCC_KEY_ACCELEROMETER_LEFT, true);
	} else {
		gbcc_input_process_key(gbc, GBCC_KEY_ACCELEROMETER_RIGHT, false);
		gbcc_input_process_key(gbc, GBCC_KEY_ACCELEROMETER_LEFT, false);
	}
	if (state[SDL_SCANCODE_KP_8]) {
		gbcc_input_process_key(gbc, GBCC_KEY_ACCELEROMETER_UP, true);
		gbcc_input_process_key(gbc, GBCC_KEY_ACCELEROMETER_DOWN, false);
	} else if (state[SDL_SCANCODE_KP_2]) {
		gbcc_input_process_key(gbc, GBCC_KEY_ACCELEROMETER_UP, false);
		gbcc_input_process_key(gbc, GBCC_KEY_ACCELEROMETER_DOWN, true);
	} else {
		gbcc_input_process_key(gbc, GBCC_KEY_ACCELEROMETER_UP, false);
		gbcc_input_process_key(gbc, GBCC_KEY_ACCELEROMETER_DOWN, false);
	}
}

int process_input(struct gbcc *gbc, const SDL_Event *e)
{
	if (e->type == SDL_QUIT) {
		gbc->quit = true;
	} else if (e->type == SDL_WINDOWEVENT) {
		if (e->window.event == SDL_WINDOWEVENT_CLOSE) {
			uint32_t id = e->window.windowID;
			if (id == SDL_GetWindowID(gbc->window.platform.window)) {
				gbc->quit = true;
				gbcc_sdl_destroy(gbc);
			} else if (id == SDL_GetWindowID(gbc->window.vram.window)) {
				gbc->window.vram_display = false;
			} else {
				gbcc_log_error("Unknown window ID %u\n", id);
			}
		} else if (e->window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
			gbc->has_focus = true;
			SDL_PumpEvents();
			SDL_FlushEvent(SDL_KEYDOWN);
		} else if (e->window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
			gbc->has_focus = false;
		}
	} else if (e->type == SDL_KEYDOWN || e->type == SDL_KEYUP) {
		for (size_t i = 0; i < N_ELEM(keymap); i++) {
			if (e->key.keysym.scancode == keymap[i]) {
				return (int)i;
			}
		}
	}
	return -1;
}
