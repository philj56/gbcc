#include "gbcc.h"
#include "gbcc_debug.h"
#include "gbcc_screenshot.h"
#include <errno.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

void gbcc_screenshot(struct gbc *gbc)
{
	char dir[] = "screenshots";
	char fname[BUFSIZ];

	time_t raw_time;
	struct tm *time_info;
	time(&raw_time);
	time_info = gmtime(&raw_time);
	strftime(fname, BUFSIZ, "screenshots/%G-%m-%dT%H:%M:%SZ.png", time_info);

	if (mkdir(dir, 0755) != 0) {
		if (errno != EEXIST) {
			gbcc_log_error("Couldn't create screenshots folder: %s\n", strerror(errno));
			return;
		}
	}
	
	FILE *fp = fopen(fname, "wbe");
	if (!fp) {
		gbcc_log_error("Couldn't open %s: %s\n", fname, strerror(errno));
		return;
	}
	png_structp png_ptr = png_create_write_struct(
			PNG_LIBPNG_VER_STRING,
			NULL, NULL, NULL);
	if (!png_ptr) {
		gbcc_log_error("Couldn't create PNG write struct.\n");
		return;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, NULL);
		fclose(fp);
		gbcc_log_error("Couldn't create PNG info struct.\n");
		return;
	}

	if (setjmp(png_jmpbuf(png_ptr)) != 0) {
		png_destroy_write_struct(&png_ptr, NULL);
		fclose(fp);
		gbcc_log_error("Couldn't setjmp for libpng.\n");
		return;
	}

	/* Initialize rows of PNG. */
	png_bytepp row_pointers = png_malloc(png_ptr, GBC_SCREEN_HEIGHT * sizeof(png_bytep));
	for (int y = 0; y < GBC_SCREEN_HEIGHT; y++) {
		png_bytep row = png_malloc(png_ptr, GBC_SCREEN_WIDTH * 3);
		row_pointers[y] = row;
		for (int x = 0; x < GBC_SCREEN_WIDTH; x++) {
			int idx = y * GBC_SCREEN_WIDTH + x;
			uint32_t pixel = gbc->memory.gbc_screen[idx];
			*row++ = (pixel & 0xFF0000u) >> 16u;
			*row++ = (pixel & 0x00FF00u) >> 8u;
			*row++ = (pixel & 0x0000FFu) >> 0u;
		}
	}

	png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr,
			GBC_SCREEN_WIDTH, GBC_SCREEN_HEIGHT,
			8, PNG_COLOR_TYPE_RGB,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
			PNG_FILTER_TYPE_DEFAULT);
	png_set_rows(png_ptr, info_ptr, row_pointers);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	for (int y = 0; y < GBC_SCREEN_HEIGHT; y++) {
		png_free(png_ptr, row_pointers[y]);
	}
	png_free(png_ptr, row_pointers);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	gbcc_log_info("Saved screenshot %s\n", fname);
}
