#include "gbcc.h"
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
 	for (uint8_t byte = 0; byte < 0x10u; byte++) {
		gbcc_memory_copy(gbc, gbc->hdma.source, gbc->hdma.dest, true);
		gbc->hdma.source++;
		gbc->hdma.dest++;
	}	
	gbc->hdma.length -= 0x10u;
	gbcc_memory_write(gbc, HDMA5, (gbc->hdma.length >> 4u) - 1, true);
}
