#include "gbcc.h"
#include "gbcc_cpu.h"
#include "gbcc_debug.h"
#include "gbcc_input.h"
#include "gbcc_window.h"
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static bool printscr = false;
static clock_t timer_start;
static clock_t timer_end;
static struct gbc gbc;

_Noreturn static void print(int sig);
void print(int sig)
{
	(void) sig;
	printscr = true;
	timer_end = clock();
	fprintf(stderr, "Speed: %f Hz\n", (double)gbc.clock / ((double)(timer_end - timer_start) / CLOCKS_PER_SEC));
	exit(0);
}

void cleanup(void)
{
}

int main(int argc, char **argv)
{
	if (argc == 1) {
		printf("Usage: %s rom_file\n", argv[0]);
		exit(1);
	}

	if (signal(SIGUSR1, print) == SIG_ERR) {
		printf("Can't catch SIGUSR1!\n");
	}

//	struct gbc gbc;

	gbcc_initialise(&gbc, argv[1]);
	gbcc_window_initialise(&gbc);

	bool loop = true;
	timer_start = clock();
	while (loop) {
		gbcc_input_process_all(&gbc);
		if (printscr && fgetc(stdin) == 'r') {
			printscr = false;
		}
		do {
			gbcc_emulate_cycle(&gbc);
		} while (gbc.instruction_timer > 0);
		if (true || printscr && gbc.instruction_timer == 0) {
			gbcc_print_registers(&gbc);
			//loop = false;
		}
	}
	
	//gbcc_free(&gbc);
	printf("END\n");
	return 0;
}
