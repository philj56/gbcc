#include "../gbcc.h"
#include "../args.h"
#include "../audio.h"
#include "../paths.h"
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
#ifdef _WIN32
	gbcc_fix_windows_path();
	signal(SIGINT, quit);
#else
	struct sigaction act = {
		.sa_handler = quit
	};
	sigfillset(&act.sa_mask);
	sigaction(SIGINT, &act, NULL);
#endif

	struct gbcc_gtk gtk = {0};
	struct gbcc *gbc = &gtk.gbc;

	gbcc_audio_initialise(gbc);
	gbcc_gtk_initialise(&gtk, &argc, &argv);

	if (!gbcc_parse_args(gbc, false, argc, argv)) {
		gbcc_audio_destroy(gbc);
		exit(EXIT_FAILURE);
	}

	if (gbc->core.initialised) {
		pthread_create(&gtk.emulation_thread, NULL, gbcc_emulation_loop, gbc);
		pthread_setname_np(gtk.emulation_thread, "EmulationThread");
	}
	gbc->has_focus = true;
	gtk_main();

	exit(EXIT_SUCCESS);
}
