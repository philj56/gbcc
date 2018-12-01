#include "gbcc.h"
#include "gbcc_apu.h"
#include "gbcc_audio.h"
#include "gbcc_cpu.h"
#include "gbcc_debug.h"
#include "gbcc_palettes.h"
#include "gbcc_save.h"
#include "gbcc_vram_window.h"
#include "gbcc_window.h"
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static struct gbc gbc;

static void quit(int sig);

__attribute__((noreturn))
void quit(int sig) 
{
	(void) sig;
	gbcc_save(&gbc);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	if (argc == 1) {
	}

	if (signal(SIGINT, quit) == SIG_ERR) {
		printf("Can't catch SIGINT!\n");
	}

	int c;
	char *pvalue = "classic";
	opterr = 0;

	while ((c = getopt(argc, argv, "p:")) != -1) {
		switch (c)
		{
			case 'p':
				pvalue = optarg;
				break;
			case '?':
				if (optopt == 'p') {
					gbcc_log(GBCC_LOG_ERROR, "Option -%c requires an argument.\n", optopt);
				} else if (isprint (optopt)) {
					gbcc_log(GBCC_LOG_ERROR, "Unknown option `-%c'.\n", optopt);
				} else {
					gbcc_log(GBCC_LOG_ERROR,
							"Unknown option character `\\x%x'.\n",
							optopt);
				}
				exit(EXIT_FAILURE);
			default:
				abort();
		}
	}
	if (argv[optind] == NULL) {
		printf("Usage: %s rom_file\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int palette = -1;	
	for (int i = sizeof(gbcc_palette_names)/sizeof(gbcc_palette_names[0]) - 1; i >= 0; i--) {
		if (strcmp(pvalue, gbcc_palette_names[i]) == 0) {
			palette = i;
			break;
		}
	}

	if (palette == -1) {
		gbcc_log(GBCC_LOG_ERROR, "Invalid palette %s\n", pvalue);
		exit(EXIT_FAILURE);
	}

	gbcc_log(GBCC_LOG_DEBUG, "Palette %s selected\n", gbcc_palette_names[palette]);	
	//	struct gbc gbc;

	/* FIXME: shouldn't have to do this */
	gbc.initialised = false;
	gbcc_initialise(&gbc, argv[optind]);
	gbcc_window_initialise(&gbc);
	//gbcc_vram_window_initialise(&gbc);
	struct gbcc_audio *audio = gbcc_audio_initialise(&gbc);
	gbcc_load(&gbc);
	gbc.initialised = true;
	if (palette == 0) {
		gbc.palette = gbcc_palettes[palette];
	} else {
		gbc.palette = gbcc_palette_correct(&gbcc_palettes[palette]);
	}

	while (!gbc.quit) {
		gbcc_emulate_cycle(&gbc);
		if (gbc.save_state > 0) {
			gbcc_save_state(&gbc);
		} else if (gbc.load_state > 0) {
			gbcc_load_state(&gbc);
		}
		gbcc_audio_update(audio);
	}
	gbcc_save(&gbc);

	return 0;
}
