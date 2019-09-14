#ifndef GBCC_GTK
#define GBCC_GTK
#endif

#include "../gbcc.h"
#include "../args.h"
#include "../audio.h"
#include "../save.h"
#include "gtk.h"
#include <gtk/gtk.h>
#include <signal.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

static void quit(int sig);

__attribute__((noreturn))
void quit(int sig) 
{
	(void) sig;
	exit(EXIT_FAILURE);
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
	gbcc_gtk_initialise(&gbc, &argc, &argv);

	if (!gbcc_parse_args(&gbc, false, argc, argv)) {
		gbcc_audio_destroy(&gbc);
		exit(EXIT_FAILURE);
	}

	if (gbc.core.initialised) {
		pthread_create(&gbc.window.platform.emulation_thread, NULL, gbcc_emulation_loop, &gbc);
		pthread_setname_np(gbc.window.platform.emulation_thread, "EmulationThread");
	}
	gbc.has_focus = true;
	gtk_main();

	exit(EXIT_SUCCESS);
}
