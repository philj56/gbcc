#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include "gbcc.h"
#include "gbcc_cpu.h"
#include "gbcc_debug.h"

bool printscr = false;

void print(int sig)
{
	printscr = true;
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

	struct gbc gbc;

	gbcc_initialise(&gbc, argv[1]);

	while (true) {
		if (printscr && fgetc(stdin) == 'r') {
			printscr = false;
		}
		do {
			gbcc_emulate_cycle(&gbc);
		} while (gbc.instruction_timer > 0);
		if (printscr && gbc.instruction_timer == 0) {
			gbcc_print_registers(&gbc);
		}
	}

	printf("END\n");
	return 0;
}
