#include "gbcc.h"

uint8_t gbcc_memory_read(struct gbc *gbc, uint16_t addr);
void gbcc_memory_write(struct gbc *gbc, uint16_t addr, uint8_t val);
