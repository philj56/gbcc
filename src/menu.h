#ifndef GBCC_MENU_H
#define GBCC_MENU_H

#include "constants.h"
#include "input.h"
#include <stdbool.h>
#include <stdint.h>

struct gbcc;

enum GBCC_MENU_ENTRY {
	GBCC_MENU_ENTRY_SAVE_STATE,
	GBCC_MENU_ENTRY_LOAD_STATE,
	GBCC_MENU_ENTRY_AUTOSAVE,
	GBCC_MENU_ENTRY_FRAMEBLEND,
	GBCC_MENU_ENTRY_LINK_CABLE,
	GBCC_MENU_ENTRY_SHADER,
	GBCC_MENU_ENTRY_TURBO_MULT
};

struct gbcc_menu {
	bool show;
	int save_state;
	int load_state;
	int width;
	int height;
	enum GBCC_MENU_ENTRY selection;
	char *text;
};

void gbcc_menu_init(struct gbcc *gbc);
void gbcc_menu_update(struct gbcc *gbc);
void gbcc_menu_process_key(struct gbcc *gbc, enum gbcc_key key);

#endif /* GBCC_MENU_H */
