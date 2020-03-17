/*
 * Copyright (C) 2017-2020 Philip Jones
 *
 * Licensed under the MIT License.
 * See either the LICENSE file, or:
 *
 * https://opensource.org/licenses/MIT
 *
 */

#include "../gbcc.h"
#include "../debug.h"
#include "../input.h"
#include "../nelem.h"
#include "../save.h"
#include "gtk.h"
#include "input.h"
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <pthread.h>

#ifndef GTK_UI_PATH
#define GTK_UI_PATH "gbcc.ui"
#endif

#ifndef ICON_PATH
#define ICON_PATH "icons"
#endif

static void on_realise(GtkGLArea *gl_area, void *data);
static gboolean on_render(GtkGLArea *gl_area, GdkGLContext *context, void *data);
static void on_vram_realise(GtkGLArea *gl_area, void *data);
static gboolean on_vram_render(GtkGLArea *gl_area, GdkGLContext *context, void *data);
static void on_destroy(GtkWidget *window, void *data);
static void on_window_state_change(GtkWidget *window, GdkEvent *event, void *data);
static void on_keypress(GtkWidget *widget, GdkEventKey *event, void *data);
static void load_rom(GtkWidget *widget, void *data);
static void stop(GtkWidget *widget, void *data);
static void save_state(GtkWidget *widget, void *data);
static void load_state(GtkWidget *widget, void *data);
static void quit(GtkWidget *widget, void *data);
static void check_savestates(GtkWidget *widget, void *data);
static void check_file_options(GtkWidget *widget, void *data);
static void check_settings_options(GtkWidget *widget, void *data);
static void check_palette(GtkWidget *widget, void *data);
static void check_shader(GtkWidget *widget, void *data);
static void check_vram_display(GtkWidget *widget, void *data);
static void change_shader(GtkWidget *widget, void *data);
static void change_palette(GtkWidget *widget, void *data);
static void toggle_vram_display(GtkCheckMenuItem *widget, void *data);
static void toggle_autosave(GtkCheckMenuItem *widget, void *data);
static void toggle_background_playback(GtkCheckMenuItem *widget, void *data);
static void toggle_fractional_scaling(GtkCheckMenuItem *widget, void *data);
static void toggle_frame_blending(GtkCheckMenuItem *widget, void *data);
static void toggle_vsync(GtkCheckMenuItem *widget, void *data);
static void toggle_interlacing(GtkCheckMenuItem *widget, void *data);
static void turbo_speed(GtkCheckMenuItem *widget, void *data);
static void show_turbo_dialog(GtkWidget *widget, void *data);
static void select_turbo_text(GtkWidget *widget, void *data);
static void custom_turbo_speed(GtkSpinButton *widget, void *data);
static void start_emulation_thread(struct gbcc_gtk *gtk, char *file);
static void stop_emulation_thread(struct gbcc_gtk *gtk);
static void gbcc_gtk_process_input(struct gbcc_gtk *gtk);
static void *init_input(void *_);

void gbcc_gtk_initialise(struct gbcc_gtk *gtk, int *argc, char ***argv)
{
	GError *error = NULL;

	gtk_init(argc, argv);

	memcpy(gtk->keymap, default_keymap, sizeof(gtk->keymap));

	gtk->icons = g_list_append(gtk->icons, gdk_pixbuf_new_from_file(ICON_PATH "icon-16x16.png", &error));
	if (error) {
		gbcc_log_error("Error loading icons: %s\n", error->message);
		g_clear_error(&error);
	}
	gtk->icons = g_list_append(gtk->icons, gdk_pixbuf_new_from_file(ICON_PATH "icon-32x32.png", &error));
	if (error) {
		gbcc_log_error("Error loading icons: %s\n", error->message);
		g_clear_error(&error);
	}
	gtk->icons = g_list_append(gtk->icons, gdk_pixbuf_new_from_file(ICON_PATH "icon-48x48.png", &error));
	if (error) {
		gbcc_log_error("Error loading icons: %s\n", error->message);
		g_clear_error(&error);
	}
	gtk->icons = g_list_append(gtk->icons, gdk_pixbuf_new_from_file(ICON_PATH "icon-64x64.png", &error));
	if (error) {
		gbcc_log_error("Error loading icons: %s\n", error->message);
		g_clear_error(&error);
	}
	gtk->icons = g_list_append(gtk->icons, gdk_pixbuf_new_from_file(ICON_PATH "icon-128x128.png", &error));
	if (error) {
		gbcc_log_error("Error loading icons: %s\n", error->message);
		g_clear_error(&error);
	}
	gtk->icons = g_list_append(gtk->icons, gdk_pixbuf_new_from_file(ICON_PATH "icon-256x256.png", &error));
	if (error) {
		gbcc_log_error("Error loading icons: %s\n", error->message);
		g_clear_error(&error);
	}

	gtk_window_set_default_icon_list(gtk->icons);
	
	GtkBuilder *builder = gtk_builder_new();
	if (gtk_builder_add_from_file(builder, GTK_UI_PATH, &error) == 0) {
		gbcc_log_error("Error loading UI description: %s\n", error->message);
		g_clear_error(&error);
		exit(EXIT_FAILURE);
	}

	gtk->window = GTK_WINDOW(gtk_builder_get_object(builder, "window"));
	g_signal_connect(G_OBJECT(gtk->window), "destroy", G_CALLBACK(on_destroy), gtk);
	g_signal_connect(G_OBJECT(gtk->window), "destroy", gtk_main_quit, NULL);
	g_signal_connect(G_OBJECT(gtk->window), "window-state-event", G_CALLBACK(on_window_state_change), gtk);

	gtk->gl_area = GTK_WIDGET(gtk_builder_get_object(builder, "gl_area"));
	gtk_widget_add_events(gtk->gl_area, GDK_KEY_PRESS_MASK);
	g_signal_connect(G_OBJECT(gtk->gl_area), "realize", G_CALLBACK(on_realise), gtk);
	g_signal_connect(G_OBJECT(gtk->gl_area), "render", G_CALLBACK(on_render), gtk);
	g_signal_connect(G_OBJECT(gtk->gl_area), "key_press_event", G_CALLBACK(on_keypress), gtk);
	g_signal_connect(G_OBJECT(gtk->gl_area), "key_release_event", G_CALLBACK(on_keypress), gtk);
	gtk_gl_area_set_required_version(GTK_GL_AREA(gtk->gl_area), 3, 2);
	
	gtk->vram_gl_area = GTK_WIDGET(gtk_builder_get_object(builder, "vram_gl_area"));
	g_signal_connect(G_OBJECT(gtk->vram_gl_area), "realize", G_CALLBACK(on_vram_realise), gtk);
	g_signal_connect(G_OBJECT(gtk->vram_gl_area), "render", G_CALLBACK(on_vram_render), gtk);
	gtk_gl_area_set_required_version(GTK_GL_AREA(gtk->vram_gl_area), 3, 2);
	
	gtk->menu.bar = GTK_WIDGET(gtk_builder_get_object(builder, "menu_bar"));
	gtk->menu.stop = GTK_WIDGET(gtk_builder_get_object(builder, "stop"));
	gtk->menu.autosave = GTK_WIDGET(gtk_builder_get_object(builder, "autosave"));
	gtk->menu.background_playback = GTK_WIDGET(gtk_builder_get_object(builder, "background_playback"));
	gtk->menu.fractional_scaling = GTK_WIDGET(gtk_builder_get_object(builder, "fractional_scaling"));
	gtk->menu.frame_blending = GTK_WIDGET(gtk_builder_get_object(builder, "frame_blending"));
	gtk->menu.vram_display = GTK_WIDGET(gtk_builder_get_object(builder, "vram_display"));
	gtk->menu.save_state.submenu = GTK_WIDGET(gtk_builder_get_object(builder, "save_state"));
	gtk->menu.load_state.submenu = GTK_WIDGET(gtk_builder_get_object(builder, "load_state"));
	gtk->menu.shader.menuitem = GTK_WIDGET(gtk_builder_get_object(builder, "shader"));
	gtk->menu.shader.submenu = GTK_WIDGET(gtk_builder_get_object(builder, "shader_menu"));
	gtk->menu.palette.menuitem = GTK_WIDGET(gtk_builder_get_object(builder, "palette"));
	gtk->menu.palette.submenu = GTK_WIDGET(gtk_builder_get_object(builder, "palette_menu"));

	gtk->turbo_dialog = GTK_DIALOG(gtk_builder_get_object(builder, "turbo_dialog"));
	gtk->menu.turbo_speed = GTK_WIDGET(gtk_builder_get_object(builder, "turbo_speed"));
	gtk->menu.turbo_custom = GTK_CHECK_MENU_ITEM(gtk_builder_get_object(builder, "turbo_custom"));
	gtk->menu.custom_turbo_speed = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "turbo_speed_val"));
	gtk->menu.vsync = GTK_WIDGET(gtk_builder_get_object(builder, "vsync"));
	gtk->menu.interlacing = GTK_WIDGET(gtk_builder_get_object(builder, "interlacing"));
	gtk_spin_button_set_range(gtk->menu.custom_turbo_speed, 0, 100);
	gtk_spin_button_set_increments(gtk->menu.custom_turbo_speed, 0.1, 1);
	g_signal_connect_swapped(gtk->turbo_dialog, "response", G_CALLBACK(gtk_widget_hide), gtk->turbo_dialog);

	{
		char name[] = "load_state_n";
		for (size_t i = 0; i < N_ELEM(gtk->menu.load_state.state); i++) {
			snprintf(name, N_ELEM(name), "save_state_%lu", i + 1);
			gtk->menu.save_state.state[i] = GTK_WIDGET(gtk_builder_get_object(builder, name));
			snprintf(name, N_ELEM(name), "load_state_%lu", i + 1);
			gtk->menu.load_state.state[i] = GTK_WIDGET(gtk_builder_get_object(builder, name));
		}
	}
	
	gtk_builder_add_callback_symbol(builder, "load", G_CALLBACK(load_rom));
	gtk_builder_add_callback_symbol(builder, "stop", G_CALLBACK(stop));
	gtk_builder_add_callback_symbol(builder, "save_state", G_CALLBACK(save_state));
	gtk_builder_add_callback_symbol(builder, "load_state", G_CALLBACK(load_state));
	gtk_builder_add_callback_symbol(builder, "quit", G_CALLBACK(quit));
	gtk_builder_add_callback_symbol(builder, "check_savestates", G_CALLBACK(check_savestates));
	gtk_builder_add_callback_symbol(builder, "check_file_options", G_CALLBACK(check_file_options));
	gtk_builder_add_callback_symbol(builder, "check_settings_options", G_CALLBACK(check_settings_options));
	gtk_builder_add_callback_symbol(builder, "check_palette", G_CALLBACK(check_palette));
	gtk_builder_add_callback_symbol(builder, "check_shader", G_CALLBACK(check_shader));
	gtk_builder_add_callback_symbol(builder, "check_vram_display", G_CALLBACK(check_vram_display));
	gtk_builder_add_callback_symbol(builder, "toggle_vram_display", G_CALLBACK(toggle_vram_display));
	gtk_builder_add_callback_symbol(builder, "toggle_autosave", G_CALLBACK(toggle_autosave));
	gtk_builder_add_callback_symbol(builder, "toggle_background_playback", G_CALLBACK(toggle_background_playback));
	gtk_builder_add_callback_symbol(builder, "toggle_fractional_scaling", G_CALLBACK(toggle_fractional_scaling));
	gtk_builder_add_callback_symbol(builder, "toggle_frame_blending", G_CALLBACK(toggle_frame_blending));
	gtk_builder_add_callback_symbol(builder, "toggle_vsync", G_CALLBACK(toggle_vsync));
	gtk_builder_add_callback_symbol(builder, "toggle_interlacing", G_CALLBACK(toggle_interlacing));
	gtk_builder_add_callback_symbol(builder, "turbo_speed", G_CALLBACK(turbo_speed));
	gtk_builder_add_callback_symbol(builder, "show_turbo_dialog", G_CALLBACK(show_turbo_dialog));
	gtk_builder_add_callback_symbol(builder, "select_turbo_text", G_CALLBACK(select_turbo_text));
	gtk_builder_add_callback_symbol(builder, "custom_turbo_speed", G_CALLBACK(custom_turbo_speed));
	gtk_builder_connect_signals(builder, gtk);

	gtk_widget_show_all(GTK_WIDGET(gtk->window));
	gtk_widget_set_visible(GTK_WIDGET(gtk->vram_gl_area), false);
	gtk_window_set_focus(gtk->window, gtk->gl_area);

	{
		/*
		 * SDL_INIT_GAMECONTROLLER is very slow, so we spin it off into
		 * its own thread here.
		 */
		pthread_t init_thread;
		pthread_create(&init_thread, NULL, init_input, NULL);
		pthread_detach(init_thread);
	}
	return;
}

void on_realise(GtkGLArea *gl_area, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	gtk_gl_area_make_current(gl_area);
	gtk_gl_area_attach_buffers(gl_area);

	gbcc_window_initialise(&gtk->gbc);
	gbcc_menu_init(&gtk->gbc);
	
	for (size_t i = 0; i < N_ELEM(gtk->menu.shader.shader); i++) {
		GtkWidget *button =  gtk_radio_menu_item_new_with_label(
				gtk->menu.shader.group,
				gtk->gbc.window.gl.shaders[i].name);
		gtk_menu_attach(GTK_MENU(gtk->menu.shader.submenu),
				button,
				0, 1, i, i + 1);
		gtk->menu.shader.group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(button));
		g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(change_shader), gtk);
		gtk_widget_show(button);
		gtk->menu.shader.shader[i] = button;
	}
	
	for (size_t i = 0; i < N_ELEM(gtk->menu.palette.palette); i++) {
		GtkWidget *button =  gtk_radio_menu_item_new_with_label(
				gtk->menu.palette.group,
				gbcc_palettes[i].name);
		gtk_menu_attach(GTK_MENU(gtk->menu.palette.submenu),
				button,
				0, 1, i, i + 1);
		gtk->menu.palette.group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(button));
		g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(change_palette), gtk);
		gtk_widget_show(button);
		gtk->menu.palette.palette[i] = button;
	}

	// Get frame clock:
	GdkGLContext *glcontext = gtk_gl_area_get_context(gl_area);
	GdkWindow *glwindow = gdk_gl_context_get_window(glcontext);
	GdkFrameClock *frame_clock = gdk_window_get_frame_clock(glwindow);

	// Connect update signal:
	g_signal_connect_swapped
		( frame_clock
		, "update"
		, G_CALLBACK(gtk_gl_area_queue_render)
		, gl_area
		) ;

	// Start updating:
	gdk_frame_clock_begin_updating(frame_clock);
}

gboolean on_render(GtkGLArea *gl_area, GdkGLContext *context, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	struct gbcc *gbc = &gtk->gbc;
	if (!gbc->core.initialised) {
		gbcc_window_clear();
		return true;
	}

	int scale = gtk_widget_get_scale_factor(GTK_WIDGET(gl_area));
	gbc->window.width = gtk_widget_get_allocated_width(GTK_WIDGET(gl_area)) * scale;
	gbc->window.height = gtk_widget_get_allocated_height(GTK_WIDGET(gl_area)) * scale;
	gbcc_window_update(gbc);
	gtk_widget_set_visible(GTK_WIDGET(gtk->vram_gl_area), gbc->window.vram_display);
	gbcc_gtk_process_input(gtk);

	return true;
}

void on_vram_realise(GtkGLArea *gl_area, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	struct gbcc *gbc = &gtk->gbc;
	gtk_gl_area_make_current(gl_area);
	gtk_gl_area_attach_buffers(gl_area);

	gbcc_vram_window_initialise(gbc);
	
	// Get frame clock:
	GdkGLContext *glcontext = gtk_gl_area_get_context(gl_area);
	GdkWindow *glwindow = gdk_gl_context_get_window(glcontext);
	GdkFrameClock *frame_clock = gdk_window_get_frame_clock(glwindow);

	// Connect update signal:
	g_signal_connect_swapped
		( frame_clock
		, "update"
		, G_CALLBACK(gtk_gl_area_queue_render)
		, gl_area
		) ;

	// Start updating:
	gdk_frame_clock_begin_updating(frame_clock);
}

gboolean on_vram_render(GtkGLArea *gl_area, GdkGLContext *context, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	struct gbcc *gbc = &gtk->gbc;
	if (!gbc->core.initialised) {
		gbcc_window_clear();
		return true;
	}

	int scale = gtk_widget_get_scale_factor(GTK_WIDGET(gl_area));
	gbc->vram_window.width = gtk_widget_get_allocated_width(GTK_WIDGET(gl_area)) * scale;
	gbc->vram_window.height = gtk_widget_get_allocated_height(GTK_WIDGET(gl_area)) * scale;
	gbcc_vram_window_update(gbc);

	return true;
}

void on_destroy(GtkWidget *window, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	struct gbcc *gbc = &gtk->gbc;
	if (gbc->core.initialised) {
		stop_emulation_thread(gtk);
		free(gtk->filename);
	}
	gbcc_audio_destroy(gbc);
}

void on_window_state_change(GtkWidget *window, GdkEvent *event, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	struct gbcc *gbc = &gtk->gbc;
	GdkWindowState state = event->window_state.new_window_state;
	gtk_widget_set_visible(gtk->menu.bar, !(state & GDK_WINDOW_STATE_FULLSCREEN));
	gbc->has_focus = state & GDK_WINDOW_STATE_FOCUSED;
}


void load_rom(GtkWidget *widget, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	GtkFileChooserNative *dialog;
	gint res;

	dialog = gtk_file_chooser_native_new("Load ROM",
                                      gtk->window,
                                      GTK_FILE_CHOOSER_ACTION_OPEN,
                                      "_Open",
                                      "_Cancel");
	{
		GtkFileFilter *filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "GameBoy ROM (*.gb/*.gbc)");
		gtk_file_filter_add_pattern(filter, "*.gb");
		gtk_file_filter_add_pattern(filter, "*.gbc");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "All Files");
		gtk_file_filter_add_pattern(filter, "*");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	}
	res = gtk_native_dialog_run(GTK_NATIVE_DIALOG(dialog));
	if (res == GTK_RESPONSE_ACCEPT) {
		char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		start_emulation_thread(gtk, filename);
		g_free(filename);
	}

	g_object_unref(dialog);
}

void stop(GtkWidget *widget, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	stop_emulation_thread(gtk);
}

void save_state(GtkWidget *widget, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	struct gbcc *gbc = &gtk->gbc;
	const gchar *name = gtk_menu_item_get_label(GTK_MENU_ITEM(widget));
	gbc->save_state = strtol(strstr(name, " _") + 2, NULL, 10);
}

void load_state(GtkWidget *widget, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	struct gbcc *gbc = &gtk->gbc;
	const gchar *name = gtk_menu_item_get_label(GTK_MENU_ITEM(widget));
	gbc->load_state = strtol(strstr(name, " _") + 2, NULL, 10);
}

void quit(GtkWidget *widget, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	gtk_widget_destroy(GTK_WIDGET(gtk->window));
}


void check_savestates(GtkWidget *widget, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	struct gbcc *gbc = &gtk->gbc;
	for (size_t i = 0; i < N_ELEM(gtk->menu.load_state.state); i++) {
		gtk_widget_set_sensitive(gtk->menu.load_state.state[i], gbcc_check_savestate(gbc, i + 1));
	}
}

void check_file_options(GtkWidget *widget, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	struct gbcc *gbc = &gtk->gbc;

	gtk_widget_set_sensitive(gtk->menu.stop, gbc->core.initialised);
	gtk_widget_set_sensitive(gtk->menu.save_state.submenu, gbc->core.initialised);
	gtk_widget_set_sensitive(gtk->menu.load_state.submenu, gbc->core.initialised);
}

void check_settings_options(GtkWidget *widget, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	struct gbcc *gbc = &gtk->gbc;

	gtk_widget_set_sensitive(gtk->menu.vram_display, gbc->core.initialised);
	gtk_widget_set_sensitive(gtk->menu.fractional_scaling, gbc->core.initialised);
	gtk_widget_set_sensitive(gtk->menu.frame_blending, gbc->core.initialised);
	gtk_widget_set_sensitive(gtk->menu.turbo_speed, gbc->core.initialised);
	gtk_widget_set_sensitive(gtk->menu.vsync, gbc->core.initialised);
	gtk_widget_set_sensitive(gtk->menu.interlacing, gbc->core.initialised);

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk->menu.autosave), gbc->autosave);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk->menu.background_playback), gbc->background_play);
	if (gbc->core.initialised) {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk->menu.fractional_scaling), gbc->window.fractional_scaling);
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk->menu.frame_blending), gbc->window.frame_blending);
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk->menu.vsync), gbc->core.sync_to_video);
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk->menu.interlacing), gbc->window.interlacing);
	}
}

void check_palette(GtkWidget *widget, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	struct gbcc *gbc = &gtk->gbc;
	if (!gbc->core.initialised || gbc->core.mode == GBC) {
		gtk_widget_set_sensitive(gtk->menu.palette.menuitem, false);
		return;
	}
	gtk_widget_set_sensitive(gtk->menu.palette.menuitem, true);
	for (size_t i = 0; i < N_ELEM(gtk->menu.palette.palette); i++) {
		GtkWidget *radio = gtk->menu.palette.palette[i];
		const char *label = gtk_menu_item_get_label(GTK_MENU_ITEM(radio));
		if (strcmp(label, gbc->core.ppu.palette.name) == 0) {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(radio), true);
			return;
		}
	}
}

void check_shader(GtkWidget *widget, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	struct gbcc *gbc = &gtk->gbc;
	const char *shader = gbc->window.gl.shaders[gbc->window.gl.cur_shader].name;
	for (size_t i = 0; i < N_ELEM(gtk->menu.shader.shader); i++) {
		GtkWidget *radio = gtk->menu.shader.shader[i];
		const char *label = gtk_menu_item_get_label(GTK_MENU_ITEM(radio));
		if (strcmp(label, shader) == 0) {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(radio), true);
			return;
		}
	}
}

void check_vram_display(GtkWidget *widget, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	int visible;
	g_object_get(gtk->vram_gl_area, "visible", &visible, NULL);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk->menu.vram_display), visible);
}

void change_shader(GtkWidget *widget, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	struct gbcc *gbc = &gtk->gbc;
	const gchar *name = gtk_menu_item_get_label(GTK_MENU_ITEM(widget));
	gbcc_window_use_shader(gbc, name);
}

void change_palette(GtkWidget *widget, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	struct gbcc *gbc = &gtk->gbc;
	const gchar *name = gtk_menu_item_get_label(GTK_MENU_ITEM(widget));
	gbc->core.ppu.palette = gbcc_get_palette(name);
}

void toggle_vram_display(GtkCheckMenuItem *widget, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	gtk->gbc.window.vram_display = gtk_check_menu_item_get_active(widget);
}

void toggle_autosave(GtkCheckMenuItem *widget, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	gtk->gbc.autosave = gtk_check_menu_item_get_active(widget);
}

void toggle_background_playback(GtkCheckMenuItem *widget, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	gtk->gbc.background_play = gtk_check_menu_item_get_active(widget);
}

void toggle_fractional_scaling(GtkCheckMenuItem *widget, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	gtk->gbc.window.fractional_scaling = gtk_check_menu_item_get_active(widget);
}

void toggle_frame_blending(GtkCheckMenuItem *widget, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	gtk->gbc.window.frame_blending = gtk_check_menu_item_get_active(widget);
}

void toggle_vsync(GtkCheckMenuItem *widget, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	gtk->gbc.core.sync_to_video = gtk_check_menu_item_get_active(widget);
}

void toggle_interlacing(GtkCheckMenuItem *widget, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	gtk->gbc.window.interlacing = gtk_check_menu_item_get_active(widget);
}

void turbo_speed(GtkCheckMenuItem *widget, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	if (!gtk_check_menu_item_get_active(widget)) {
		return;
	}
	const gchar *name = gtk_menu_item_get_label(GTK_MENU_ITEM(widget));
	if (!strncmp(name, "_Unlimited", 9)) {
		gtk->gbc.core.turbo_speed = 0;
	} else {
		gtk->gbc.core.turbo_speed = strtod(strstr(name, "_") + 1, NULL);
		if (gtk->gbc.core.turbo_speed == 0) {
			gtk->gbc.core.turbo_speed = 10;
		}
	}
}

void show_turbo_dialog(GtkWidget *widget, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(gtk->menu.turbo_custom))) {
		return;
	}
	gtk_dialog_run(gtk->turbo_dialog);
}

void select_turbo_text(GtkWidget *widget, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	gtk_editable_select_region(GTK_EDITABLE(gtk->menu.custom_turbo_speed), 0, -1);
}

void custom_turbo_speed(GtkSpinButton *widget, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	float val = 0;
	g_object_get(widget, "value", &val, NULL);
	gtk->gbc.core.turbo_speed = val;
}

void start_emulation_thread(struct gbcc_gtk *gtk, char *file)
{
	struct gbcc *gbc = &gtk->gbc;
	if (gbc->core.initialised) {
		stop_emulation_thread(gtk);
		free(gtk->filename);
	}
	gbc->quit = false;
	size_t len = strlen(file) + 1;
	gtk->filename = malloc(len);
	strncpy(gtk->filename, file, len);
	gbcc_initialise(&gbc->core, gtk->filename);
	pthread_create(&gtk->emulation_thread, NULL, gbcc_emulation_loop, gbc);
	pthread_setname_np(gtk->emulation_thread, "EmulationThread");
}

void stop_emulation_thread(struct gbcc_gtk *gtk)
{
	struct gbcc *gbc = &gtk->gbc;
	if (!gbc->core.initialised) {
		return;
	}
	gbc->quit = true;
	sem_post(&gbc->core.ppu.vsync_semaphore);
	pthread_join(gtk->emulation_thread, NULL);
	gbc->save_state = 0;
	gbcc_save_state(gbc);
	gbcc_free(&gbc->core);
	gbc->core = (struct gbcc_core){0};
}

void on_keypress(GtkWidget *widget, GdkEventKey *event, void *data)
{
	struct gbcc_gtk *gtk = (struct gbcc_gtk *)data;
	struct gbcc *gbc = &gtk->gbc;
	bool val = false;
	if (event->type == GDK_KEY_PRESS) {
		val = true;
	}
	if (val && event->keyval == GDK_KEY_F11) {
		gtk->fullscreen = !gtk->fullscreen;
		if (gtk->fullscreen) {
			gtk_window_fullscreen(gtk->window);
		} else {
			gtk_window_unfullscreen(gtk->window);
		}
	}
	for (size_t i = 0; i < N_ELEM(gtk->keymap); i++) {
		if (event->keyval == gtk->keymap[i]) {
			if (!(event->state & GDK_SHIFT_MASK)) {
				gbcc_input_process_key(gbc, i, val);
				break;
			}
			switch (i) {
				case GBCC_KEY_LOAD_STATE_1:
				case GBCC_KEY_LOAD_STATE_2:
				case GBCC_KEY_LOAD_STATE_3:
				case GBCC_KEY_LOAD_STATE_4:
				case GBCC_KEY_LOAD_STATE_5:
				case GBCC_KEY_LOAD_STATE_6:
				case GBCC_KEY_LOAD_STATE_7:
				case GBCC_KEY_LOAD_STATE_8:
				case GBCC_KEY_LOAD_STATE_9:
					gbcc_input_process_key(gbc, i - 9, val);
					break;
				default:
					gbcc_input_process_key(gbc, i, val);
					break;
			}
		}
	}
}

static const SDL_GameControllerButton buttonmap[8] = {
	SDL_CONTROLLER_BUTTON_B,
	SDL_CONTROLLER_BUTTON_A,
	SDL_CONTROLLER_BUTTON_START,
	SDL_CONTROLLER_BUTTON_BACK,
	SDL_CONTROLLER_BUTTON_DPAD_UP,
	SDL_CONTROLLER_BUTTON_DPAD_DOWN,
	SDL_CONTROLLER_BUTTON_DPAD_LEFT,
	SDL_CONTROLLER_BUTTON_DPAD_RIGHT
};

// Returns key that changed, or -1 for a non-emulated key
static int process_input(struct gbcc_gtk *gtk, const SDL_Event *e);
static void process_game_controller(struct gbcc_gtk *gtk);

void gbcc_gtk_process_input(struct gbcc_gtk *gtk)
{
	struct gbcc *gbc = &gtk->gbc;
	int jx = SDL_GameControllerGetAxis(gtk->game_controller, SDL_CONTROLLER_AXIS_LEFTX);
	int jy = SDL_GameControllerGetAxis(gtk->game_controller, SDL_CONTROLLER_AXIS_LEFTY);
	int ax = SDL_GameControllerGetAxis(gtk->game_controller, SDL_CONTROLLER_AXIS_RIGHTX);
	int ay = SDL_GameControllerGetAxis(gtk->game_controller, SDL_CONTROLLER_AXIS_RIGHTY);
	SDL_Event e;
	while (SDL_PollEvent(&e) != 0) {
		int key = process_input(gtk, &e);
		enum gbcc_key emulator_key;
		bool val;
		if (e.type == SDL_KEYDOWN || e.type == SDL_CONTROLLERBUTTONDOWN) {
			val = true;
		} else if (e.type == SDL_KEYUP || e.type == SDL_CONTROLLERBUTTONUP) {
			val = false;
		} else if (e.type == SDL_CONTROLLERAXISMOTION) {
			if (e.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY) {
				gbc->core.cart.mbc.accelerometer.real_y = 0x81D0u - (0x70 * ay / 32768.0);
			} else if (e.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX) {
				gbc->core.cart.mbc.accelerometer.real_x = 0x81D0u - (0x70 * ax / 32768.0);
			}
			if (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
				val = (abs(e.caxis.value) > abs(jx));
			} else if (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
				val = (abs(e.caxis.value) > abs(jy));
			} else {
				continue;
			}
		} else {
			continue;
		}

		switch(key) {
			case 0:
				emulator_key = GBCC_KEY_A;
				break;
			case 1:
				emulator_key = GBCC_KEY_B;
				break;
			case 2:
				emulator_key = GBCC_KEY_START;
				break;
			case 3:
				emulator_key = GBCC_KEY_SELECT;
				break;
			case 4:
				emulator_key = GBCC_KEY_UP;
				break;
			case 5:
				emulator_key = GBCC_KEY_DOWN;
				break;
			case 6:
				emulator_key = GBCC_KEY_LEFT;
				break;
			case 7:
				emulator_key = GBCC_KEY_RIGHT;
				break;
			default:
				continue;
		}
		if (e.type == SDL_CONTROLLERAXISMOTION) {
			if (e.caxis.value == 0) {
				if (emulator_key == GBCC_KEY_DOWN) {
					gbcc_input_process_key(gbc, GBCC_KEY_UP, false);
				} else if (emulator_key == GBCC_KEY_RIGHT) {
					gbcc_input_process_key(gbc, GBCC_KEY_LEFT, false);
				}
			}
		}
		gbcc_input_process_key(gbc, emulator_key, val);
	}
	process_game_controller(gtk);
	gbcc_input_accelerometer_update(&gbc->core.cart.mbc.accelerometer);
}

int process_input(struct gbcc_gtk *gtk, const SDL_Event *e)
{
	if (e->type == SDL_CONTROLLERBUTTONDOWN || e->type == SDL_CONTROLLERBUTTONUP) {
		for (size_t i = 0; i < N_ELEM(buttonmap); i++) {
			if (e->cbutton.button == buttonmap[i]) {
				return (int)i;
			}
		}
	} else if (e->type == SDL_CONTROLLERAXISMOTION) {
		if (e->caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
			if (e->caxis.value < 0) {
				return 4;
			} else {
				return 5;
			}
		} else if (e->caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
			if (e->caxis.value < 0) {
				return 6;
			} else {
				return 7;
			}
		}
	}
	return -1;
}

void process_game_controller(struct gbcc_gtk *gtk)
{
	if (gtk->game_controller != NULL && !SDL_GameControllerGetAttached(gtk->game_controller)) {
		gbcc_log_info("Controller \"%s\" disconnected.\n", SDL_GameControllerName(gtk->game_controller));
		SDL_HapticClose(gtk->haptic);
		SDL_GameControllerClose(gtk->game_controller);
		gtk->game_controller = NULL;
	} 
	if (gtk->game_controller == NULL) {
		for (int i = 0; i < SDL_NumJoysticks(); i++) {
			if (SDL_IsGameController(i)) {
				gtk->game_controller = SDL_GameControllerOpen(i);
				gbcc_log_info("Controller \"%s\" connected.\n", SDL_GameControllerName(gtk->game_controller));
				break;
			}
		}
		if (gtk->game_controller == NULL || SDL_NumHaptics() == 0) {
			return;
		}
		gtk->haptic = SDL_HapticOpenFromJoystick(SDL_GameControllerGetJoystick(gtk->game_controller));
		if (SDL_HapticRumbleInit(gtk->haptic) != 0) {
			printf("%s\n", SDL_GetError());
			return;
		}
	}

	if (gtk->gbc.core.cart.rumble_state) {
		if (SDL_HapticRumblePlay(gtk->haptic, 0.1, 1000) != 0) {
			printf("%s\n", SDL_GetError());
		}
	} else {
		if (SDL_HapticRumbleStop(gtk->haptic) != 0) {
			printf("%s\n", SDL_GetError());
		}
	}
}

void *init_input(void *_)
{
	if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) != 0) {
		gbcc_log_error("Failed to initialize controller support: %s\n", SDL_GetError());
	}
	return NULL;
}
