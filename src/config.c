/*
 * Copyright (C) 2017-2020 Philip Jones
 *
 * Licensed under the MIT License.
 * See either the LICENSE file, or:
 *
 * https://opensource.org/licenses/MIT
 *
 */

#include "core.h"
#include "config.h"
#include "debug.h"
#include "save.h"
#include "window.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *strip(const char *str);
static void parse_option(struct gbcc *gbc, const char *option, const char *value);
static char *get_config_path(void);

void gbcc_load_config(struct gbcc *gbc, char *filename)
{
	bool default_filename = false;
	if (!filename) {
		default_filename = true;
		filename = get_config_path();
	}
	char *config;
	int size;
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		if (errno == ENOENT) {
			if (default_filename) {
				free(filename);
			}
			return;
		}
		gbcc_log_error("Failed to open config file %s\n", filename);
		if (default_filename) {
			free(filename);
		}
		return;
	}
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	config = malloc(size + 1);
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
	config[size] = '\0';
	
	gbcc_log_info("Loading config from %s...\n", filename);
	
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
		parse_option(gbc, option, value);
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
void parse_option(struct gbcc *gbc, const char *option, const char *value)
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

	if (strcmp(opt, "autoresume") == 0) {
		if (strtol(val, NULL, 0)) {
			gbcc_load_state(gbc);
		}
	} else if (strcmp(opt, "background") == 0) {
		gbc->background_play = strtol(val, NULL, 0);
	} else if (strcmp(opt, "fractional") == 0) {
		gbc->window.fractional_scaling = strtol(val, NULL, 0);
	} else if (strcmp(opt, "interlacing") == 0) {
		gbc->window.interlacing = strtol(val, NULL, 0);
	} else if (strcmp(opt, "palette") == 0) {
		gbc->core.ppu.palette = gbcc_get_palette(val);
	} else if (strcmp(opt, "shader") == 0) {
		gbcc_window_use_shader(gbc, val);
	} else if (strcmp(opt, "turbo") == 0) {
		gbc->core.turbo_speed = strtod(val, NULL);
	} else if (strcmp(opt, "vsync") == 0) {
	} else if (strcmp(opt, "vram-window") == 0) {
		gbc->window.vram_display = strtol(val, NULL, 0);
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
