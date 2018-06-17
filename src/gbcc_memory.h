#ifndef GBCC_MEMORY_H
#define GBCC_MEMORY_H

#include "gbcc.h"

void gbcc_memory_increment(struct gbc *gbc, uint16_t addr, bool override);
void gbcc_memory_copy(struct gbc *gbc, uint16_t src, uint16_t dest, bool override);
void gbcc_memory_set_bit(struct gbc *gbc, uint16_t addr, uint8_t b, bool override);
void gbcc_memory_clear_bit(struct gbc *gbc, uint16_t addr, uint8_t b, bool override);

uint8_t gbcc_memory_read(struct gbc *gbc, uint16_t addr, bool override);
void gbcc_memory_write(struct gbc *gbc, uint16_t addr, uint8_t val, bool override);

#endif /* GBCC_MEMORY_H */
