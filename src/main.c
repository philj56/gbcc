#include "gbcc.h"
#include "gbcc_cpu.h"
#include "gbcc_debug.h"
#include "gbcc_window.h"
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static size_t offset = 0;
static clock_t timer_start;
static struct gbc gbc;

static void print_speed(int sig);
void print_speed(int sig)
{
	(void) sig;
	double speed = (double)(gbc.clock - offset) / ((double)(clock() - timer_start) / CLOCKS_PER_SEC);
	fprintf(stderr, "Speed: %.0fHz (%.0f%%)\n", speed, 100 * speed / GBC_CLOCK_FREQ);
	timer_start = clock();
	offset = gbc.clock;
	if (signal(SIGUSR1, print_speed) == SIG_ERR) {
		printf("Can't catch SIGUSR1!\n");
	}
}

int main(int argc, char **argv)
{
	if (argc == 1) {
		printf("Usage: %s rom_file\n", argv[0]);
		exit(1);
	}

	if (signal(SIGUSR1, print_speed) == SIG_ERR) {
		printf("Can't catch SIGUSR1!\n");
	}

//	struct gbc gbc;

	gbcc_initialise(&gbc, argv[1]);
	gbcc_window_initialise(&gbc);

	bool loop = true;
	timer_start = clock();
	while (loop) {
		gbcc_emulate_cycle(&gbc);
	}
	
	//gbcc_free(&gbc);
	printf("END\n");
	return 0;
}
