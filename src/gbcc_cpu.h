#ifndef GBCC_CPU_H
#define GBCC_CPU_H

#include "gbcc.h"

uint8_t gbcc_fetch_instruction(struct gbc *gbc);
void gbcc_emulate_cycle(struct gbc *gbc);

#endif /* GBCC_CPU_H */
