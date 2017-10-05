#include "gbcc.h"

uint8_t gbcc_read_memory(struct gbc *gbc, uint16_t addr)
{
	if (addr < ROMX_START + ROMX_SIZE 
			|| (addr > SRAM_START && addr < SRAM_START + SRAM_SIZE)) {
		/* TODO: MBC selector switch statement */
		return 0;
	}
	return 0;
}

void gbcc_write_memory(struct gbc *gbc, uint16_t addr)
{

}
