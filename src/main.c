#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include "gbcc.h"
#include "gbcc_cpu.h"

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
		gbcc_emulate_cycle(&gbc);
		if (printscr) {
			printf("Test\n");
			exit(1);
		}
	}

	return 0;
}
