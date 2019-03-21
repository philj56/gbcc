#include "gbcc.h"
#include "config.h"
#include "debug.h"
#include "window.h"

static void parse_option(struct gbcc_window *win, const char *option, const char *value);
static char *get_config_path();

void gbcc_load_config(struct gbcc_window *win, char *filename)
{
	bool default_filename = false;
	if (!filename) {
		default_filename = true;
		filename = get_config_path();
	}
	gbcc_log_info("Loading config from %s...\n", filename);
	char *config;
	int size;
	FILE *fp = fopen(filename, "rbe");
	if (!fp) {
		gbcc_log_error("Failed to open config file %s\n", filename);
		if (default_filename) {
			free(filename);
		}
		return;
	}
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	config = malloc(size);
	if (!config) {
		gbcc_log_error("Failed to malloc buffer for %s\n", filename);
		fclose(fp);
		if (default_filename) {
			free(filename);
		}
		return;
	}
	rewind(fp);
	fread(config, 1, size, fp);
	fclose(fp);
	
	char *str1;
	char *line;
	char *option;
	char *value;
	char *saveptr1;
	char *saveptr2;

	for (str1 = config; ; str1 = NULL) {
		line = strtok_r(str1, "\r\n", &saveptr1);
		if (!line) {
			break;
		}
		if (line[0] == '#') {
			continue;
		}
		option = strtok_r(line, " \t", &saveptr2);
		if (!option) {
			continue;
		}
		value = strtok_r(NULL, " \t", &saveptr2);
		if (!value) {
			continue;
		}
		parse_option(win, option, value);
	}
	free(config);
	if (default_filename) {
		free(filename);
	}
}

/* TODO: Lots */
void parse_option(struct gbcc_window *win, const char *option, const char *value)
{
	if (strcmp(option, "fractional") == 0) {
		win->fractional_scaling = strtol(value, NULL, 0);
		return;
	}
	if (strcmp(option, "interlace") == 0) {
		win->gbc->interlace = strtol(value, NULL, 0);
		return;
	}
	if (strcmp(option, "palette") == 0) {
		win->gbc->ppu.palette = gbcc_get_palette(value);
		return;
	}
	if (strcmp(option, "subpixel") == 0) {
	}
	if (strcmp(option, "turbo") == 0) {
		win->gbc->turbo_speed = strtod(value, NULL);
		return;
	}
	if (strcmp(option, "vsync") == 0) {
		return;
	}
	if (strcmp(option, "vram-window") == 0) {
		return;
	}
	gbcc_log_warning("\tBad config file option \"%s\"\n", option);
}

char *get_config_path()
{
	char *base_dir = getenv("XDG_CONFIG_HOME");
	char *ext = "";
	size_t len = strlen("/gbcc/config") + 1;
	if (!base_dir) {
		base_dir = getenv("HOME");
		ext = "/.config";
		if (!base_dir) {
			gbcc_log_error("Couldn't find XDG_CONFIG_HOME or HOME envvars\n");
			return NULL;
		}
	}
	len += strlen(base_dir) + strlen(ext);
	char *name = calloc(len, sizeof(char));
	sprintf(name, "%s%s%s", base_dir, ext, "/gbcc/config");
	return name;
}
