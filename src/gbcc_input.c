#include "gbcc.h"
#include "gbcc_input.h"
#include "gbcc_save.h"
#include "gbcc_window.h"

static const SDL_Keycode keymap[11] = {
	SDLK_z,		/* A */
	SDLK_x, 	/* B */
	SDLK_RETURN,	/* Start */
	SDLK_SPACE,	/* Select */
	SDLK_UP,	/* DPAD up */
	SDLK_DOWN,	/* DPAD down */
	SDLK_LEFT,	/* DPAD left */
	SDLK_RIGHT,	/* DPAD right */
	SDLK_RSHIFT, 	/* Turbo */
	SDLK_1,		/* Load State */
	SDLK_2		/* Save State */
};

// Returns key that changed, or -1 for a non-emulated key
static int gbcc_input_process(struct gbc *gbc, const SDL_Event *e);

void gbcc_input_process_all(struct gbc *gbc)
{
	SDL_Event e;
	while (SDL_PollEvent(&e) != 0) {
		int key = gbcc_input_process(gbc, &e);
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
				gbc->load_state = val;
				break;
			case 10:
				gbc->save_state = val;
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
			if (e->key.keysym.sym == keymap[i]) {
				return (int)i;
			}
		}
	}
	return -1;
}
