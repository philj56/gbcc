#include "gbcc.h"
#include "gbcc_apu.h"
#include "gbcc_audio.h"
#include "gbcc_cpu.h"
#include "gbcc_debug.h"
#include "gbcc_palettes.h"
#include "gbcc_save.h"
#include "gbcc_vram_window.h"
#include "gbcc_window.h"
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static void quit(int sig);
static void usage(void);

__attribute__((noreturn))
void quit(int sig) 
{
	(void) sig;
	exit(EXIT_FAILURE);
}

void usage()
{
	printf("Usage: gbcc [-hiv] [-p palette] [-t speed] rom_file\n"
	       "  -h, --help            Print this message and exit.\n"
	       "  -i, --interlace       Enable interlacing (experimental).\n"
	       "  -p, --palette=NAME    Select the colour palette (DMG mode only).\n"
	       "  -t, --turbo=NUM    	Set a fractional speed limit for turbo mode\n"
	       "                        (0 = unlimited).\n"
	       "  -v, --vsync           Enable VSync.\n"
	      );
}

static struct gbc gbc;

int main(int argc, char **argv)
{
	struct sigaction act = {
		.sa_handler = quit
	};
	sigfillset(&act.sa_mask);
	sigaction(SIGINT, &act, NULL);

	bool vsync = false;
	opterr = 0;

	struct option long_options[] = {
		{"help", no_argument, NULL, 'h'},
		{"interlace", no_argument, NULL, 'i'},
		{"palette", required_argument, NULL, 'p'},
		{"turbo", required_argument, NULL, 't'},
		{"vsync", no_argument, NULL, 'v'}
	};

	while (getopt_long(argc, argv, "hip:t:v", long_options, NULL) != -1) {
	}
	if (optind >= argc) {
		usage();
		exit(EXIT_FAILURE);
	}
	gbcc_initialise(&gbc, argv[optind]);

	optind = 1;

	for (int opt; (opt = getopt_long(argc, argv, "hip:t:v", long_options, NULL)) != -1;) {
		switch (opt) {
			case 'h':
				usage();
				exit(EXIT_SUCCESS);
			case 'i':
				gbc.interlace = true;
				break;
			case 't':
				/* TODO: error check */
				gbc.turbo_speed = strtod(optarg, NULL);
				break;
			case 'p':
				gbc.palette = gbcc_get_palette(optarg);
				gbcc_log_debug("%s palette selected\n", gbc.palette.name);
				break;
			case 'v':
				vsync = true;
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
	gbcc_window_initialise(&gbc, vsync);
	//gbcc_vram_window_initialise(&gbc);
	struct gbcc_audio *audio = gbcc_audio_initialise(&gbc);
	gbcc_load(&gbc);

	while (!gbc.quit) {
		while (gbc.pause) {
			const struct timespec time = {.tv_sec = 0, .tv_nsec = 10000000};
			nanosleep(&time, NULL);
		}
		gbcc_emulate_cycle(&gbc);
		if (gbc.save_state > 0) {
			gbcc_save_state(&gbc);
		} else if (gbc.load_state > 0) {
			gbcc_load_state(&gbc);
		}
		gbcc_audio_update(audio);
	}
	gbcc_save(&gbc);

	exit(EXIT_SUCCESS);
}
