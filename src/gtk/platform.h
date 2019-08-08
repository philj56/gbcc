#ifndef GBCC_PLATFORM_H
#define GBCC_PLATFORM_H

#include "../palettes.h"
#include <gtk/gtk.h>
#include <pthread.h>
#include <SDL2/SDL.h>

struct gbcc_platform {
	pthread_t emulation_thread;
	SDL_GameController *game_controller;
	SDL_Haptic *haptic;
	GtkWindow *window;
	GtkWidget *gl_area;
	struct {
		GtkWidget *bar;
		GtkWidget *stop;
		GtkWidget *vram_display;
		struct {
			GtkWidget *submenu;
			GtkWidget *state[9];
		} save_state;
		struct {
			GtkWidget *submenu;
			GtkWidget *state[9];
		} load_state;
		struct {
			GtkWidget *menuitem;
			GtkWidget *submenu;
			GSList *group;
			GtkWidget *shader[4];
		} shader;
		struct {
			GtkWidget *menuitem;
			GtkWidget *submenu;
			GSList *group;
			GtkWidget *palette[GBCC_NUM_PALETTES];
		} palette;
	} menu;
	char *filename;
};

#endif /* GBCC_PLATFORM_H */
