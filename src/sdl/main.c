#include "../gbcc.h"
#include "../apu.h"
#include "../args.h"
#include "../audio.h"
#include "../config.h"
#include "../cpu.h"
#include "../debug.h"
#include "../memory.h"
#include "../palettes.h"
#include "../save.h"
#include "../time.h"
#include "sdl.h"
#include "vram_window.h"
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

static bool force_quit;

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
	struct sigaction act = {
		.sa_handler = quit
	};
	sigfillset(&act.sa_mask);
	sigaction(SIGINT, &act, NULL);

	struct gbcc gbc = {0};
	gbcc_audio_initialise(&gbc);
	gbcc_sdl_initialise(&gbc);

	if (!gbcc_parse_args(&gbc, true, argc, argv)) {
		gbcc_audio_destroy(&gbc);
		exit(EXIT_FAILURE);
	}

	pthread_t emu_thread;
	pthread_create(&emu_thread, NULL, gbcc_emulation_loop, &gbc);
	pthread_setname_np(emu_thread, "EmulationThread");

	struct timespec t1;
	struct timespec t2;
	int time_to_sleep;
	while (!gbc.quit && !force_quit) {
		clock_gettime(CLOCK_REALTIME, &t2);
		gbcc_sdl_update(&gbc);
		gbcc_sdl_process_input(&gbc);
		clock_gettime(CLOCK_REALTIME, &t1);
		time_to_sleep = 8 - (int)(gbcc_time_diff(&t1, &t2) / 1e6);
		if (time_to_sleep > 0 && time_to_sleep < 16) {
			SDL_Delay(time_to_sleep);
		}
	}
	if (force_quit) {
		gbcc_audio_destroy(&gbc);
		exit(EXIT_FAILURE);
	}
	pthread_join(emu_thread, NULL);
	gbcc_audio_destroy(&gbc);

	gbc.save_state = 0;
	gbcc_save_state(&gbc);

	exit(EXIT_SUCCESS);
}
