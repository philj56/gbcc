#include "../gbcc.h"
#include "../debug.h"
#include "../input.h"
#include "../nelem.h"
#include "../window.h"
#include "sdl.h"
#include "vram_window.h"
#include <math.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

static const SDL_Scancode keymap[33] = {
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
	SDL_SCANCODE_A, 	/* Toggle autosave */
	SDL_SCANCODE_B, 	/* Toggle background playback */
	SDL_SCANCODE_ESCAPE, 	/* Toggle menu */
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

const SDL_GameControllerButton buttonmap[8] = {
	SDL_CONTROLLER_BUTTON_B,
	SDL_CONTROLLER_BUTTON_A,
	SDL_CONTROLLER_BUTTON_START,
	SDL_CONTROLLER_BUTTON_BACK,
	SDL_CONTROLLER_BUTTON_DPAD_UP,
	SDL_CONTROLLER_BUTTON_DPAD_DOWN,
	SDL_CONTROLLER_BUTTON_DPAD_LEFT,
	SDL_CONTROLLER_BUTTON_DPAD_RIGHT
};

// Returns key that changed, or -1 for a non-emulated key
static int process_input(struct gbcc_sdl *sdl, const SDL_Event *e);
static void process_game_controller(struct gbcc_sdl *sdl);
static void *init_input(void *_);

void gbcc_sdl_initialise(struct gbcc_sdl *sdl)
{
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
		gbcc_log_error("Failed to initialize SDL: %s\n", SDL_GetError());
	}

	sdl->game_controller = NULL;

	{
		/*
		 * SDL_INIT_GAMECONTROLLER is very slow, so we spin it off into
		 * its own thread here.
		 */
		pthread_t init_thread;
		pthread_create(&init_thread, NULL, init_input, NULL);
		pthread_detach(init_thread);
	}
	
	/* Main OpenGL settings */
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);


	sdl->window = SDL_CreateWindow(
			"GBCC",                    // window title
			SDL_WINDOWPOS_UNDEFINED,   // initial x position
			SDL_WINDOWPOS_UNDEFINED,   // initial y position
			GBC_SCREEN_WIDTH,      // width, in pixels
			GBC_SCREEN_HEIGHT,     // height, in pixels
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE // flags
			);

	if (sdl->window == NULL) {
		gbcc_log_error("Could not create window: %s\n", SDL_GetError());
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	sdl->context = SDL_GL_CreateContext(sdl->window);
	SDL_GL_MakeCurrent(sdl->window, sdl->context);

	gbcc_window_initialise(&sdl->gbc);
	gbcc_menu_init(&sdl->gbc);
}

void gbcc_sdl_destroy(struct gbcc_sdl *sdl)
{
	if (sdl->game_controller != NULL) {
		SDL_GameControllerClose(sdl->game_controller);
		sdl->game_controller = NULL;
	}
	SDL_GL_MakeCurrent(sdl->window, sdl->context);
	gbcc_window_deinitialise(&sdl->gbc);
	SDL_GL_DeleteContext(sdl->context);
	SDL_DestroyWindow(sdl->window);
}

void gbcc_sdl_update(struct gbcc_sdl *sdl)
{
	struct gbcc *gbc = &sdl->gbc;
	struct gbcc_window *win = &gbc->window;
	if (win->vram_display) {
		if (!gbc->vram_window.initialised) {
			gbcc_sdl_vram_window_initialise(sdl);

			/* 
			 * Workaround to allow input on vram window.
			 * The VRAM window immediately grabs input focus,
			 * causing an SDL_KEYDOWN to fire, and closing the
			 * window instantly :). To stop that, we make sure that
			 * the main window has focus, and then discard any
			 * SDL_KEYDOWN events.
			 */
			SDL_RaiseWindow(sdl->window);
			SDL_PumpEvents();
			SDL_FlushEvent(SDL_KEYDOWN);
		}
		gbcc_sdl_vram_window_update(sdl);
	} else if (gbc->vram_window.initialised) {
		gbcc_sdl_vram_window_destroy(sdl);
	}

	SDL_GL_MakeCurrent(sdl->window, sdl->context);
	SDL_GL_GetDrawableSize(sdl->window, &win->width, &win->height);
	gbcc_window_update(gbc);
	SDL_GL_SwapWindow(sdl->window);
}

void gbcc_sdl_process_input(struct gbcc_sdl *sdl)
{
	struct gbcc *gbc = &sdl->gbc;
	int jx = SDL_GameControllerGetAxis(sdl->game_controller, SDL_CONTROLLER_AXIS_LEFTX);
	int jy = SDL_GameControllerGetAxis(sdl->game_controller, SDL_CONTROLLER_AXIS_LEFTY);
	SDL_Event e;
	const uint8_t *state = SDL_GetKeyboardState(NULL);
	while (SDL_PollEvent(&e) != 0) {
		int key = process_input(sdl, &e);
		enum gbcc_key emulator_key;
		bool val;
		if (e.type == SDL_KEYDOWN || e.type == SDL_CONTROLLERBUTTONDOWN) {
			val = true;
		} else if (e.type == SDL_KEYUP || e.type == SDL_CONTROLLERBUTTONUP) {
			val = false;
		} else if (e.type == SDL_CONTROLLERAXISMOTION) {
			if (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
				val = (abs(e.caxis.value) > abs(jx));
			} else if (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
				val = (abs(e.caxis.value) > abs(jy));
			} else {
				continue;
			}
		} else {
			continue;
		}

		switch(key) {
			case -2:
				return;
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
				if (state[SDL_SCANCODE_LSHIFT]) {
					emulator_key = GBCC_KEY_FRAME_BLENDING;
				} else {
					emulator_key = GBCC_KEY_FPS;
				}
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
				emulator_key = GBCC_KEY_AUTOSAVE;
				break;
			case 18:
				emulator_key = GBCC_KEY_BACKGROUND_PLAY;
				break;
			case 19:
				emulator_key = GBCC_KEY_MENU;
				break;
			case 20:
				if (state[SDL_SCANCODE_LSHIFT]) {
					emulator_key = GBCC_KEY_ACCELEROMETER_MAX_DOWN;
				} else {
					continue;
				}
				break;
			case 21:
				if (state[SDL_SCANCODE_LSHIFT]) {
					emulator_key = GBCC_KEY_ACCELEROMETER_MAX_LEFT;
				} else {
					continue;
				}
				break;
			case 22:
				if (state[SDL_SCANCODE_LSHIFT]) {
					emulator_key = GBCC_KEY_ACCELEROMETER_MAX_RIGHT;
				} else {
					continue;
				}
				break;
			case 23:
				if (state[SDL_SCANCODE_LSHIFT]) {
					emulator_key = GBCC_KEY_ACCELEROMETER_MAX_UP;
				} else {
					continue;
				}
				break;
			case 24:
			case 25:
			case 26:
			case 27:
			case 28:
			case 29:
			case 30:
			case 31:
			case 32:
				if (state[SDL_SCANCODE_LSHIFT]) {
					emulator_key = GBCC_KEY_SAVE_STATE_1 + (int8_t)(key - 22);
				} else {
					emulator_key = GBCC_KEY_LOAD_STATE_1 + (int8_t)(key - 22);
				}
				break;
			default:
				continue;
		}
		if (e.type == SDL_CONTROLLERAXISMOTION) {
			if (e.caxis.value == 0) {
				if (emulator_key == GBCC_KEY_DOWN) {
					gbcc_input_process_key(gbc, GBCC_KEY_UP, false);
				} else if (emulator_key == GBCC_KEY_RIGHT) {
					gbcc_input_process_key(gbc, GBCC_KEY_LEFT, false);
				}
			}
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
	process_game_controller(sdl);
	gbcc_input_accelerometer_update(&gbc->core.cart.mbc.accelerometer);
}

int process_input(struct gbcc_sdl *sdl, const SDL_Event *e)
{
	struct gbcc *gbc = &sdl->gbc;
	if (e->type == SDL_QUIT) {
		gbc->quit = true;
	} else if (e->type == SDL_WINDOWEVENT) {
		if (e->window.event == SDL_WINDOWEVENT_CLOSE) {
			uint32_t id = e->window.windowID;
			if (id == SDL_GetWindowID(sdl->window)) {
				gbc->quit = true;
				gbcc_sdl_destroy(sdl);
				return -2;
			} else if (id == SDL_GetWindowID(sdl->vram_window)) {
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
	} else if (e->type == SDL_CONTROLLERBUTTONDOWN || e->type == SDL_CONTROLLERBUTTONUP) {
		for (size_t i = 0; i < N_ELEM(buttonmap); i++) {
			if (e->cbutton.button == buttonmap[i]) {
				return (int)i;
			}
		}
	} else if (e->type == SDL_CONTROLLERAXISMOTION) {
		if (e->caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
			if (e->caxis.value < 0) {
				return 4;
			} else {
				return 5;
			}
		} else if (e->caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
			if (e->caxis.value < 0) {
				return 6;
			} else {
				return 7;
			}
		}
	}
	return -1;
}

void process_game_controller(struct gbcc_sdl *sdl)
{
	if (sdl->game_controller != NULL && !SDL_GameControllerGetAttached(sdl->game_controller)) {
		gbcc_log_info("Controller \"%s\" disconnected.\n", SDL_GameControllerName(sdl->game_controller));
		SDL_GameControllerClose(sdl->game_controller);
		sdl->game_controller = NULL;
	} 
	if (sdl->game_controller == NULL) {
		for (int i = 0; i < SDL_NumJoysticks(); i++) {
			if (SDL_IsGameController(i)) {
				sdl->game_controller = SDL_GameControllerOpen(i);
				gbcc_log_info("Controller \"%s\" connected.\n", SDL_GameControllerName(sdl->game_controller));
				break;
			}
		}
		sdl->haptic = SDL_HapticOpenFromJoystick(SDL_GameControllerGetJoystick(sdl->game_controller));
		if (sdl->game_controller == NULL || SDL_NumHaptics() == 0) {
			return;
		}
		if (SDL_HapticRumbleInit(sdl->haptic) != 0) {
			printf("%s\n", SDL_GetError());
			return;
		}
	}

	if (sdl->gbc.core.cart.rumble_state) {
		if (SDL_HapticRumblePlay(sdl->haptic, 0.1, 100) != 0) {
			printf("%s\n", SDL_GetError());
		}
	} else {
		if (SDL_HapticRumbleStop(sdl->haptic) != 0) {
			printf("%s\n", SDL_GetError());
		}
	}
}

void *init_input(void *_)
{
	if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) != 0) {
		gbcc_log_error("Failed to initialize controller support: %s\n", SDL_GetError());
	}
	return NULL;
}
