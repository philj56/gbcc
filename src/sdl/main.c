/*
 * Copyright (C) 2017-2020 Philip Jones
 *
 * Licensed under the MIT License.
 * See either the LICENSE file, or:
 *
 * https://opensource.org/licenses/MIT
 *
 */

#include "../gbcc.h"
#include "../apu.h"
#include "../args.h"
#include "../audio.h"
#include "../camera.h"
#include "../cheats.h"
#include "../config.h"
#include "../cpu.h"
#include "../debug.h"
#include "../memory.h"
#include "../palettes.h"
#include "../paths.h"
#include "../save.h"
#include "../time_diff.h"
#include "sdl.h"
#include "vram_window.h"
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static bool force_quit;
static struct gbcc_sdl sdl = {0};

static void quit(int sig);

void quit(int sig) 
{
	(void) sig;
	if (force_quit) {
		exit(EXIT_FAILURE);
	}
	force_quit = true;
}

int main(int argc, char **argv)
{
#ifdef _WIN32
	gbcc_fix_windows_path();
	signal(SIGINT, quit);
#else
	struct sigaction act = {
		.sa_handler = quit
	};
	sigfillset(&act.sa_mask);
	sigaction(SIGINT, &act, NULL);
#endif

	struct gbcc *gbc = &sdl.gbc;
	gbcc_audio_initialise(gbc, 96000, 2048);
	gbcc_sdl_initialise(&sdl);

	if (!gbcc_parse_args(gbc, true, argc, argv)) {
		gbcc_audio_destroy(gbc);
		exit(EXIT_FAILURE);
	}

	gbcc_camera_initialise(gbc);

	pthread_t emu_thread;
	pthread_create(&emu_thread, NULL, gbcc_emulation_loop, gbc);
	pthread_setname_np(emu_thread, "EmulationThread");

	while (!gbc->quit && !force_quit) {
		gbcc_sdl_update(&sdl);
		gbcc_sdl_process_input(&sdl);
	}
	sem_post(&gbc->core.ppu.vsync_semaphore);
	if (!force_quit) {
		pthread_join(emu_thread, NULL);
	}
	gbcc_audio_destroy(gbc);
	gbcc_camera_destroy(gbc);
	if (force_quit) {
		exit(EXIT_FAILURE);
	}

	gbc->save_state = 0;
	gbcc_save_state(gbc);

	exit(EXIT_SUCCESS);
}
