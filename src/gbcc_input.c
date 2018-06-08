#include "gbcc.h"
#include "gbcc_input.h"
#include "gbcc_window.h"

static const SDL_Keycode keymap[16] = {
	SDLK_x,	// 0
	SDLK_1,	// 1
	SDLK_2,	// 2
	SDLK_3,	// 3
	SDLK_q,	// 4
	SDLK_w,	// 5
	SDLK_e,	// 6
	SDLK_a,	// 7
	SDLK_s,	// 8
	SDLK_d,	// 9
	SDLK_z,	// A
	SDLK_c,	// B
	SDLK_4,	// C
	SDLK_r,	// D
	SDLK_f,	// E
	SDLK_v	// F
};

// Returns key that changed, or -1 for a non-emulated key
int gbcc_input_process(const SDL_Event *e);

void gbcc_input_process_all(struct gbc *gbc)
{
	SDL_Event e;
	while (SDL_PollEvent(&e) != 0) {
		int key = gbcc_input_process(&e);
		if (key < 0) {
			continue;
		}
		if (e.type == SDL_KEYDOWN) {
			/* TODO: Handle keypresses */
		} else {
		}
	}
}

int gbcc_input_process(const SDL_Event *e)
{
	if (e->type == SDL_QUIT) {
		exit(0);
	} else if (e->type == SDL_KEYDOWN || e->type == SDL_KEYUP) {
		for (int i = 0; i < 16; i++) {
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
