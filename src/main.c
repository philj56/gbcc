#include "gbcc.h"
#include "gbcc_audio.h"
#include "gbcc_cpu.h"
#include "gbcc_debug.h"
#include "gbcc_save.h"
#include "gbcc_window.h"
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static struct gbc gbc;
static struct gbcc_window *window;

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
		printf("Usage: %s rom_file\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (signal(SIGINT, quit) == SIG_ERR) {
		printf("Can't catch SIGINT!\n");
	}

//	struct gbc gbc;

	gbc.initialised = false;
	gbcc_initialise(&gbc, argv[1]);
	window = gbcc_window_initialise(&gbc);
	gbcc_audio_initialise();
	gbcc_load(&gbc);
	gbc.initialised = true;

	while (!gbc.quit) {
		gbcc_emulate_cycle(&gbc);
	}
	gbcc_save(&gbc);
	
	return 0;
}
