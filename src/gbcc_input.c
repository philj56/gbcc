#include "gbcc.h"
#include "gbcc_input.h"
#include "gbcc_save.h"
#include "gbcc_window.h"

static const SDL_Keycode keymap[9] = {
	SDLK_z,		/* A */
	SDLK_x, 	/* B */
	SDLK_RETURN,	/* Start */
	SDLK_SPACE,	/* Select */
	SDLK_UP,	/* DPAD up */
	SDLK_DOWN,	/* DPAD down */
	SDLK_LEFT,	/* DPAD left */
	SDLK_RIGHT,	/* DPAD right */
	SDLK_RSHIFT 	/* Turbo */
};

// Returns key that changed, or -1 for a non-emulated key
static int gbcc_input_process(const SDL_Event *e);

void gbcc_input_process_all(struct gbc *gbc)
{
	SDL_Event e;
	while (SDL_PollEvent(&e) != 0) {
		if (e.type == SDL_WINDOWEVENT) {
			if (e.window.event == SDL_WINDOWEVENT_CLOSE) {
				printf("Window Closed\n");
				/* TODO: Move this somewhere more sensible */
				//gbcc_save(gbc);
			}
		}
		int key = gbcc_input_process(&e);
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
		}
	}
}

int gbcc_input_process(const SDL_Event *e)
{
	if (e->type == SDL_QUIT) {
		exit(0);
	} else if (e->type == SDL_KEYDOWN || e->type == SDL_KEYUP) {
		for (int i = 0; i < 9; i++) {
			if (e->key.keysym.sym == keymap[i]) {
				return i;
			}
		}
	}
	return -1;
}

int gbcc_input_wait()
{
	SDL_Event e;
	int key;

	SDL_WaitEvent(&e);
	while ((key = gbcc_input_process(&e)) < 0 || e.type != SDL_KEYDOWN)
	{
		SDL_WaitEvent(&e);
	}
	return key;
}
