#include "gbcc.h"
#include "gbcc_hdma.h"
#include "gbcc_memory.h"

void gbcc_hdma_copy(struct gbc *gbc)
{
	uint16_t start = gbc->hdma.source;
	uint16_t end = gbc->hdma.source + gbc->hdma.length;
	uint16_t dest = gbc->hdma.dest;
	for (uint16_t src = start; src < end; src++, dest++) {
		gbcc_memory_copy(gbc, src, dest, true);
	}
	gbcc_memory_write(gbc, HDMA5, 0xFFu, true);
	gbc->hdma.length = 0;
}

void gbcc_hdma_copy_block(struct gbc *gbc)
{
	if (gbc->hdma.length <= 0) {
		return;
	}
 	for (uint8_t byte = 0; byte < 0x10u && gbc->hdma.length > 0; byte++) {
		gbcc_memory_copy(gbc, gbc->hdma.source, gbc->hdma.dest, true);
		gbc->hdma.source++;
		gbc->hdma.dest++;
		gbc->hdma.length--;
	}	
	gbcc_memory_write(gbc, HDMA1, (gbc->hdma.source >> 8u), true);
	gbcc_memory_write(gbc, HDMA2, (gbc->hdma.source & 0xFFu), true);
	gbcc_memory_write(gbc, HDMA3, (gbc->hdma.dest >> 8u), true);
	gbcc_memory_write(gbc, HDMA4, (gbc->hdma.dest & 0xFFu), true);
	if (gbc->hdma.length == 0) {
		gbcc_memory_write(gbc, HDMA5, 0xFFu, true);
	} else {
		gbcc_memory_write(gbc, HDMA5, (uint8_t)((gbc->hdma.length >> 4u) - 1), true);
	}
}
