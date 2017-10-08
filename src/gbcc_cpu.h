#ifndef GBCC_CPU_H
#define GBCC_CPU_H

#include "gbcc.h"

uint8_t gbcc_fetch_instruction(struct gbc *gbc);
void gbcc_emulate_cycle(struct gbc *gbc);
void gbcc_add_instruction_cycles(struct gbc *gbc, uint8_t cycles);

#endif /* GBCC_CPU_H */
