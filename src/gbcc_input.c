#include "gbcc.h"
#include "gbcc_input.h"
#include "gbcc_save.h"
#include "gbcc_window.h"

static const SDL_Scancode keymap[20] = {
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
static int gbcc_input_process(struct gbc *gbc, const SDL_Event *e);

void gbcc_input_process_all(struct gbc *gbc)
{
	SDL_Event e;
	while (SDL_PollEvent(&e) != 0) {
		int key = gbcc_input_process(gbc, &e);
		const uint8_t *state = SDL_GetKeyboardState(NULL);
		bool val;
		if (key < 0) {
			continue;
		}
		if (e.type == SDL_KEYDOWN) {
			val = true;
			gbc->halt.set = false;
			gbc->stop = false;
		} else {
			val = false;
		}
		switch(key) {
			case 0:
				gbc->keys.a = val;
				break;
			case 1:
				gbc->keys.b = val;
				break;
			case 2:
				gbc->keys.start = val;
				break;
			case 3:
				gbc->keys.select = val;
				break;
			case 4:
				gbc->keys.dpad.up = val;
				break;
			case 5:
				gbc->keys.dpad.down = val;
				break;
			case 6:
				gbc->keys.dpad.left = val;
				break;
			case 7:
				gbc->keys.dpad.right = val;
				break;
			case 8:
				gbc->keys.turbo ^= val;
				break;
			case 9:
				gbc->screenshot = val;
				break;
			case 10:
				gbc->pause ^= val;
				break;
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
			case 18:
			case 19:
				if (!val) {
					break;
				}
				if (state[SDL_SCANCODE_LSHIFT]) {
					gbc->save_state = (int8_t)(key - 10);
				} else {
					gbc->load_state = (int8_t)(key - 10);
				}
				break;
		}
	}
}

int gbcc_input_process(struct gbc *gbc, const SDL_Event *e)
{
	if (e->type == SDL_QUIT) {
		gbc->quit = true;
	} else if (e->type == SDL_KEYDOWN || e->type == SDL_KEYUP) {
		for (size_t i = 0; i < sizeof(keymap) / sizeof(keymap[0]); i++) {
			if (e->key.keysym.scancode == keymap[i]) {
				return (int)i;
			}
		}
	}
	return -1;
}
