#include "gbcc_memory.h"

uint8_t gbcc_read_memory(struct gbc *gbc, uint16_t addr)
{
	return *gbcc_get_memory_ref(gbc, addr);
}

void gbcc_write_memory(struct gbc *gbc, uint16_t addr, uint8_t val)
{
	*gbcc_get_memory_ref(gbc, addr) = val;
}

uint8_t *gbcc_get_memory_ref(struct gbc *gbc, uint16_t addr)
{
	if (addr < ROMX_START + ROMX_SIZE 
			|| (addr > SRAM_START && addr < SRAM_START + SRAM_SIZE)) {
		/* TODO: MBC selector switch statement */
	}
	return NULL;
}
