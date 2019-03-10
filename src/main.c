#include "gbcc.h"
#include "apu.h"
#include "audio.h"
#include "cpu.h"
#include "debug.h"
#include "input.h"
#include "memory.h"
#include "palettes.h"
#include "save.h"
#include "time.h"
#include "vram_window.h"
#include "window.h"
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>

static void quit(int sig);
static void usage(void);
static int emulation_loop(void *_audio);

__attribute__((noreturn))
void quit(int sig) 
{
	(void) sig;
	exit(EXIT_FAILURE);
}

void usage()
{
	printf("Usage: gbcc [-fhisvV] [-p palette] [-t speed] rom_file\n"
	       "  -f, --fractional      Enable fractional scaling.\n"
	       "  -h, --help            Print this message and exit.\n"
	       "  -i, --interlace       Enable interlacing (experimental).\n"
	       "  -p, --palette=NAME    Select the colour palette (DMG mode only).\n"
	       "  -s, --subpixel        Enable subpixel scaling.\n"
	       "  -t, --turbo=NUM    	Set a fractional speed limit for turbo mode\n"
	       "                        (0 = unlimited).\n"
	       "  -v, --vsync           Enable VSync (currently ineffective).\n"
	       "  -V, --vram-window     Display a window with all vram tile data.\n"
	      );
}

int emulation_loop(void *_audio)
{
	struct gbcc_audio *audio = (struct gbcc_audio *)_audio;
	struct gbc *gbc = audio->gbc;
	gbcc_load(gbc);
	while (!gbc->quit) {
		while (gbc->pause) {
			const struct timespec time = {.tv_sec = 0, .tv_nsec = 10000000};
			nanosleep(&time, NULL);
		}
		gbcc_emulate_cycle(gbc);
		if (gbc->load_state > 0) {
			gbcc_load_state(gbc);
		} else if (gbc->save_state > 0) {
			gbcc_save_state(gbc);
		}
		gbcc_audio_update(audio);
	}
	gbcc_save(gbc);
	return 0;
}

int main(int argc, char **argv)
{
	struct sigaction act = {
		.sa_handler = quit
	};
	sigfillset(&act.sa_mask);
	sigaction(SIGINT, &act, NULL);

	struct gbc gbc;
	struct gbcc_audio audio = {0};
	struct gbcc_window win = {0};
	struct gbcc_vram_window vwin = {0};
	bool vram_window = false;
	opterr = 0;

	struct option long_options[] = {
		{"fractional", no_argument, NULL, 'f'},
		{"help", no_argument, NULL, 'h'},
		{"interlace", no_argument, NULL, 'i'},
		{"palette", required_argument, NULL, 'p'},
		{"subpixel", no_argument, NULL, 's'},
		{"turbo", required_argument, NULL, 't'},
		{"vsync", no_argument, NULL, 'v'},
		{"vram-window", no_argument, NULL, 'V'}
	};
	const char *short_options = "fhip:st:vV";

	for (int opt; (opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1;) {
		if (opt == 'h') {
			usage();
			exit(EXIT_SUCCESS);
		}
	}
	if (optind >= argc) {
		usage();
		exit(EXIT_FAILURE);
	}
	gbcc_initialise(&gbc, argv[optind]);
	gbcc_audio_initialise(&audio, &gbc);
	gbcc_window_initialise(&win, &gbc);
	SDL_Init(0);

	optind = 1;

	for (int opt; (opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1;) {
		switch (opt) {
			case 'f':
				win.fractional_scaling = true;
				break;
			case 'h':
				usage();
				exit(EXIT_SUCCESS);
			case 'i':
				gbc.interlace = true;
				break;
			case 'p':
				gbc.ppu.palette = gbcc_get_palette(optarg);
				gbcc_log_debug("%s palette selected\n", gbc.ppu.palette.name);
				break;
			case 's':
				win.gl.cur_shader = 1;
				break;
			case 't':
				/* TODO: error check */
				gbc.turbo_speed = strtod(optarg, NULL);
				break;
			case 'v':
				SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
				break;
			case 'V':
				vram_window = true;
				break;
			case '?':
				if (optopt == 'p' || optopt == 't') {
					gbcc_log_error("Option -%c requires an argument.\n", optopt);
				} else if (isprint(optopt)) {
					gbcc_log_error("Unknown option `-%c'.\n", optopt);
				} else {
					gbcc_log_error("Unknown option character `\\x%x'.\n", optopt);
				}
				usage();
				exit(EXIT_FAILURE);
			default:
				usage();
				exit(EXIT_FAILURE);
		}
	}

	if (vram_window) {
		gbcc_vram_window_initialise(&vwin, &gbc);
	}

	thrd_t emu_thread;
	thrd_create(&emu_thread, emulation_loop, &audio);

	struct timespec t1;
	struct timespec t2;
	int time_to_sleep;
	while (!gbc.quit) {
		clock_gettime(CLOCK_REALTIME, &t2);
		gbcc_input_process_all(&win);
		gbcc_window_update(&win);
		if (vram_window) {
			gbcc_vram_window_update(&vwin);
		}
		clock_gettime(CLOCK_REALTIME, &t1);
		time_to_sleep = 8 - (int)(gbcc_time_diff(&t1, &t2) / 1e6);
		if (time_to_sleep > 0) {
			SDL_Delay(time_to_sleep);
		}
	}
	thrd_join(emu_thread, NULL);

	exit(EXIT_SUCCESS);
}
