#ifndef GBCC_FONTMAP_H
#define GBCC_FONTMAP_H

#include <stdint.h>

struct gbcc_fontmap {
	uint32_t tile_width;
	uint32_t tile_height;
	uint8_t *bitmap;
};

void gbcc_fontmap_load(struct gbcc_fontmap *fontmap, const char *filename);
void gbcc_fontmap_destroy(struct gbcc_fontmap *fontmap);

#endif /* GBCC_FONTMAP_H */
