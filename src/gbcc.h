#ifndef GBCC_H
#define GBCC_H

#include "audio.h"
#include "core.h"
#include "window.h"
#include "vram_window.h"

struct gbcc {
	struct gbcc_core core;
	struct gbcc_window window;
	struct gbcc_vram_window vram_window;
	struct gbcc_audio audio;
	
	bool quit;
	bool pause;
	bool interlace;
	int8_t save_state;
	int8_t load_state;
	bool background_play;
	bool has_focus;
	bool quiet;
};

void *gbcc_emulation_loop(void *_gbc);

#endif /* GBCC_H */
