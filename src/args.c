/*
 * Copyright (C) 2017-2020 Philip Jones
 *
 * Licensed under the MIT License.
 * See either the LICENSE file, or:
 *
 * https://opensource.org/licenses/MIT
 *
 */

#include "args.h"
#include "config.h"
#include "debug.h"
#include "gbcc.h"
#include "save.h"
#include <ctype.h>
#include <getopt.h>
#include <stdio.h>

static void usage()
{
	printf("Usage: gbcc [-aAbfFhivV] [-c config_file] [-p palette] [-s shader] [-t speed] rom\n"
	       "  -a, --autoresume      Automatically resume gameplay if possible.\n"
	       "  -A, --autosave        Automatically save SRAM after last write.\n"
	       "  -b, --background      Enable playback while unfocused.\n"
	       "  -c, --config=PATH     Path to custom config file.\n"
	       "  -f, --fractional      Enable fractional scaling.\n"
	       "  -F, --frame-blending  Enable simple frame blending.\n"
	       "  -h, --help            Print this message and exit.\n"
	       "  -i, --interlacing       Enable interlacing.\n"
	       "  -p, --palette=NAME    Select the colour palette (DMG mode only).\n"
	       "  -s, --shader=NAME     Select the initial shader to use.\n"
	       "  -t, --turbo=NUM    	Set a fractional speed limit for turbo mode\n"
	       "                        (0 = unlimited).\n"
	       "  -v, --vsync           Enable VSync (experimental).\n"
	       "  -V, --vram-window     Display a window with all vram tile data.\n"
	      );
}

bool gbcc_parse_args(struct gbcc *gbc, bool file_required, int argc, char **argv)
{
	opterr = 0;

	struct option long_options[] = {
		{"autoresume", no_argument, NULL, 'a'},
		{"autosave", no_argument, NULL, 'A'},
		{"background", no_argument, NULL, 'b'},
		{"config", required_argument, NULL, 'c'},
		{"fractional", no_argument, NULL, 'f'},
		{"frame-blending", no_argument, NULL, 'F'},
		{"help", no_argument, NULL, 'h'},
		{"interlacing", no_argument, NULL, 'i'},
		{"palette", required_argument, NULL, 'p'},
		{"shader", required_argument, NULL, 's'},
		{"turbo", required_argument, NULL, 't'},
		{"vsync", no_argument, NULL, 'v'},
		{"vram-window", no_argument, NULL, 'V'}
	};
	const char *short_options = "aAbc:fFhip:s:t:vV";

	for (int opt; (opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1;) {
		if (opt == 'h') {
			usage();
			return false;
		}
	}
	if (optind >= argc) {
		if (file_required) {
			usage();
			return false;
		}
	} else {
		gbcc_initialise(&gbc->core, argv[optind]);
		if (gbc->core.error) {
			return false;
		}
	}

	char *config = NULL;
	optind = 1;
	for (int opt; (opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1;) {
		if (opt == 'c') {
			config = optarg;
		}
	}
	gbcc_load_config(gbc, config);

	optind = 1;
	for (int opt; (opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1;) {
		switch (opt) {
			case 'a':
				gbcc_load_state(gbc);
				break;
			case 'A':
				gbc->autosave = true;
				break;
			case 'b':
				gbc->background_play = true;
				break;
			case 'c':
				break;
			case 'f':
				gbc->fractional_scaling = true;
				break;
			case 'F':
				gbc->frame_blending = true;
				break;
			case 'h':
				usage();
				return false;
			case 'i':
				gbc->interlacing = true;
				break;
			case 'p':
				gbc->core.ppu.palette = gbcc_get_palette(optarg);
				gbcc_log_debug("%s palette selected\n", gbc->core.ppu.palette.name);
				break;
			case 's':
				gbcc_window_use_shader(gbc, optarg);
				break;
			case 't':
				/* TODO: error check */
				gbc->turbo_speed = strtod(optarg, NULL);
				break;
			case 'v':
				gbc->core.sync_to_video = true;
				break;
			case 'V':
				gbc->vram_display = true;
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
				return false;
			default:
				usage();
				return false;
		}
	}

	return true;
}
