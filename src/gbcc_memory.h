#include "gbcc.h"

uint8_t gbcc_read_memory(struct gbc *gbc, uint16_t addr);
void gbcc_write_memory(struct gbc *gbc, uint16_t addr, uint8_t val);
uint8_t *gbcc_get_memory_ref(struct gbc *gbc, uint16_t addr);
