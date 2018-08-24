#include "gbcc.h"
#include "gbcc_apu.h"
#include "gbcc_audio.h"
#include "gbcc_cpu.h"
#include "gbcc_debug.h"
#include "gbcc_save.h"
#include "gbcc_window.h"
#include "gbcc_constants.h"
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
		printf("Usage: %s rom_file\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (signal(SIGINT, quit) == SIG_ERR) {
		printf("Can't catch SIGINT!\n");
	}

	int c;
	char *pvalue = "brown";
	opterr = 0;

	while ((c = getopt(argc, argv, "p:")) != -1)
	    switch (c)
	    {
		case 'p':
		    pvalue = optarg;
		    break;
		case '?':
		    if (optopt == 'c')
			fprintf (stderr, "Option -%c requires an argument.\n", optopt);
		    else if (isprint (optopt))
			fprintf (stderr, "Unknown option `-%c'.\n", optopt);
		    else
			fprintf (stderr,
				"Unknown option character `\\x%x'.\n",
				optopt);
		    return 1;
		default:
		    abort ();
	    }
	for (int j = 0;j<argc;j++){
	    printf("arg %d is %s\n", j,argv[j]);
	}
	    
	char* PALETTE_NAME[] = { "brown", "red", "darkbrown", NULL };
	enum PALETTE_TYPE pnum;
	
	printf("test = %s\n",argv[argc-1]);
	printf("pvalue = %s\n", pvalue);

	pnum = brown;
	int i=0;
	for (i=0; PALETTE_NAME[i]!=NULL; ++i, ++pnum)
	    if (0==strcmp(pvalue, PALETTE_NAME[i])) break;

	printf("pnum = %d\n", pnum);
	
//	struct gbc gbc;

	/* FIXME: shouldn't have to do this */
	gbc.initialised = false;
	gbcc_initialise(&gbc, argv[argc-1]);
	gbcc_window_initialise(&gbc);
	struct gbcc_audio *audio = gbcc_audio_initialise(&gbc);
	gbcc_load(&gbc);
	gbc.initialised = true;
	gbc.palette = pnum;
	printf("gbc.palette = %d\n", gbc.palette);
	while (!gbc.quit) {
		gbcc_emulate_cycle(&gbc);
		if (gbc.save_state > 0) {
			gbcc_save_state(&gbc);
		} else if (gbc.load_state > 0) {
			gbcc_load_state(&gbc);
		}
		gbcc_audio_update(audio);
	}
	//gbcc_vram_dump(&gbc, "vram.dump");
	gbcc_save(&gbc);
	
	return 0;
}
