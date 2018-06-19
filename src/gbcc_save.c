#include "gbcc.h"
#include "gbcc_debug.h"
#include "gbcc_save.h"
#include <stdio.h>

#define MAX_NAME_LEN 256

void gbcc_save(struct gbc *gbc)
{
	if (gbc->cart.ram_size == 0) {
		return;
	}
	char fname[MAX_NAME_LEN];
	FILE *sav;
	snprintf(fname, MAX_NAME_LEN, "%s.sav", gbc->cart.title);
	gbcc_log(GBCC_LOG_INFO, "Saving %s...\n", fname);
	sav = fopen(fname, "wbe");
	if (sav == NULL) {
		gbcc_log(GBCC_LOG_ERROR, "Can't open save file %s\n", fname);
		exit(1);
	}
	fwrite(gbc->cart.ram, 1, gbc->cart.ram_size, sav);
	fclose(sav);
	gbcc_log(GBCC_LOG_INFO, "Saved.\n");
}

void gbcc_load(struct gbc *gbc)
{
	char fname[MAX_NAME_LEN];
	FILE *sav;
	snprintf(fname, MAX_NAME_LEN, "%s.sav", gbc->cart.title);
	sav = fopen(fname, "rbe");
	if (sav == NULL) {
		return;
	}
	gbcc_log(GBCC_LOG_INFO, "Loading %s...\n", fname);
	fread(gbc->cart.ram, 1, gbc->cart.ram_size, sav);
	fclose(sav);
}
