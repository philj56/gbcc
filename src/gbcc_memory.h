#ifndef GBCC_MEMORY_H
#define GBCC_MEMORY_H

#include "gbcc.h"

uint8_t gbcc_memory_read(struct gbc *gbc, uint16_t addr);
void gbcc_memory_write(struct gbc *gbc, uint16_t addr, uint8_t val);

#endif /* GBCC_MEMORY_H */
