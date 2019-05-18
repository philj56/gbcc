#include "gbcc.h"
#include "config.h"
#include "debug.h"
#include "window.h"
#include <stdlib.h>
#include <string.h>

static char *strip(const char *str);
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
		option = strtok_r(line, "=", &saveptr2);
		if (!option) {
			gbcc_log_warning("\tBad config line \"%s\"\n", line);
			continue;
		}
		value = strtok_r(NULL, "\r\n", &saveptr2);
		if (!value) {
			gbcc_log_warning("\tBad config line \"%s\"\n", line);
			continue;
		}
		parse_option(win, option, value);
	}
	free(config);
	if (default_filename) {
		free(filename);
	}
}

char *strip(const char *str)
{
	char *buf = calloc(strlen(str) + 1, 1);
	int start = 0;
	int end = strlen(str) - 1;
	while (isspace(str[start])) {
		start++;
	}
	while (isspace(str[end])) {
		end--;
	}
	if (end < start) {
		free(buf);
		return NULL;
	}
	strncpy(buf, str + start, (end + 1) - start);
	return buf;
}

/* TODO: Lots */
void parse_option(struct gbcc_window *win, const char *option, const char *value)
{
	char *opt = strip(option);
	if (!opt) {
		gbcc_log_warning("\tBad config line \"%s=%s\"\n", option, value);
		return;
	}

	char *val = strip(value);
	if (!val) {
		gbcc_log_warning("\tBad config line \"%s=%s\"\n", option, value);
		free(opt);
		return;
	}

	if (strcmp(opt, "background") == 0) {
		win->gbc->background_play = strtol(val, NULL, 0);
	} else if (strcmp(opt, "fractional") == 0) {
		win->fractional_scaling = strtol(val, NULL, 0);
	} else if (strcmp(opt, "interlace") == 0) {
		win->gbc->interlace = strtol(val, NULL, 0);
	} else if (strcmp(opt, "palette") == 0) {
		win->gbc->ppu.palette = gbcc_get_palette(val);
	} else if (strcmp(opt, "shader") == 0) {
		gbcc_window_use_shader(win, val);
	} else if (strcmp(opt, "turbo") == 0) {
		win->gbc->turbo_speed = strtod(val, NULL);
	} else if (strcmp(opt, "vsync") == 0) {
	} else if (strcmp(opt, "vram-window") == 0) {
		win->vram_display = strtol(val, NULL, 0);
	} else {
		gbcc_log_warning("\tBad config file option \"%s\"\n", opt);
	}
	free(opt);
	free(val);
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
