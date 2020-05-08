#include "camera.h"
#include "../debug.h"
#include <png.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifndef CAMERA_PATH
#define CAMERA_PATH "camera.png"
#endif

#define HEADER_BYTES 8

void gbcc_camera_platform_capture_image(uint8_t image[GB_CAMERA_SENSOR_SIZE]) {
	static bool initialised = false;
	static uint8_t im[GB_CAMERA_SENSOR_SIZE];

	if (initialised) {
		memcpy(image, im, GB_CAMERA_SENSOR_SIZE);
		return;
	}

	FILE *fp = fopen(CAMERA_PATH, "rb");
	uint8_t header[HEADER_BYTES];
	if (!fp) {
		gbcc_log_error("Couldn't open %s\n", CAMERA_PATH);
		return;
	}
	if (fread(header, 1, HEADER_BYTES, fp) == 0) {
		gbcc_log_error("Failed to read fontmap data: %s\n", CAMERA_PATH);
		fclose(fp);
		return;
	}
	if (png_sig_cmp(header, 0, HEADER_BYTES)) {
		gbcc_log_error("Not a PNG file: %s\n", CAMERA_PATH);
		fclose(fp);
		return;
	}

	png_structp png_ptr = png_create_read_struct(
			PNG_LIBPNG_VER_STRING,
			NULL, NULL, NULL);
	if (!png_ptr) {
		gbcc_log_error("Couldn't create PNG read struct.\n");
		fclose(fp);
		return;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		fclose(fp);
		gbcc_log_error("Couldn't create PNG info struct.\n");
		return;
	}

	if (setjmp(png_jmpbuf(png_ptr)) != 0) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		gbcc_log_error("Couldn't setjmp for libpng.\n");
		return;
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, HEADER_BYTES);
	png_read_info(png_ptr, info_ptr);

	uint32_t width = png_get_image_width(png_ptr, info_ptr);
	uint32_t height = png_get_image_height(png_ptr, info_ptr);

	if (width != GB_CAMERA_SENSOR_WIDTH || height != GB_CAMERA_SENSOR_HEIGHT) {
		gbcc_log_error("Invalid camera image dimensions.\n");
		exit(EXIT_FAILURE);
	}

	uint32_t bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	
	png_bytepp row_pointers = calloc(GB_CAMERA_SENSOR_HEIGHT, sizeof(png_bytep));
	for (uint32_t y = 0; y < GB_CAMERA_SENSOR_HEIGHT; y++) {
		row_pointers[y] = (unsigned char *)&im[y * GB_CAMERA_SENSOR_WIDTH];
	}

	if (bit_depth < 8) {
		png_set_packing(png_ptr);
	}
	png_read_image(png_ptr, row_pointers);
	png_read_end(png_ptr, NULL);

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	free(row_pointers);
	fclose(fp);

	memcpy(image, im, GB_CAMERA_SENSOR_SIZE);
	initialised = true;
	return;
}
