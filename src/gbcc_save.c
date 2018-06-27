#include "gbcc.h"
#include "gbcc_debug.h"
#include "gbcc_save.h"
#include <stdio.h>
#include <string.h>

#define MAX_NAME_LEN 4096

static void strip_ext(char *fname);

void gbcc_save(struct gbc *gbc)
{
	if (gbc->cart.ram_size == 0) {
		return;
	}
	char fname[MAX_NAME_LEN];
	char tmp[MAX_NAME_LEN];
	FILE *sav;
	if (snprintf(tmp, MAX_NAME_LEN, "%s", gbc->cart.filename) >= MAX_NAME_LEN) {
		gbcc_log(GBCC_LOG_ERROR, "Filename %s too long\n", tmp);
	}
	strip_ext(tmp);
	if (snprintf(fname, MAX_NAME_LEN, "%s.sav", tmp) >= MAX_NAME_LEN) {
		gbcc_log(GBCC_LOG_ERROR, "Filename %s too long\n", fname);
	}
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
	char tmp[MAX_NAME_LEN];
	FILE *sav;
	if (snprintf(tmp, MAX_NAME_LEN, "%s", gbc->cart.filename) >= MAX_NAME_LEN) {
		gbcc_log(GBCC_LOG_ERROR, "Filename %s too long\n", tmp);
	}
	strip_ext(tmp);
	if (snprintf(fname, MAX_NAME_LEN, "%s.sav", tmp) >= MAX_NAME_LEN) {
		gbcc_log(GBCC_LOG_ERROR, "Filename %s too long\n", fname);
	}
	sav = fopen(fname, "rbe");
	if (sav == NULL) {
		return;
	}
	gbcc_log(GBCC_LOG_INFO, "Loading %s...\n", fname);
	fread(gbc->cart.ram, 1, gbc->cart.ram_size, sav);
	fclose(sav);
}

void strip_ext(char *fname)
{
	char *end = fname + strlen(fname);
	while (end > fname && *end != '.') {
		--end;
	}

	if (end > fname) {
		*end = '\0';
	}
}
