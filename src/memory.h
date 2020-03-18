/*
 * Copyright (C) 2017-2020 Philip Jones
 *
 * Licensed under the MIT License.
 * See either the LICENSE file, or:
 *
 * https://opensource.org/licenses/MIT
 *
 */

#ifndef GBCC_MEMORY_H
#define GBCC_MEMORY_H

#include "core.h"
#include <stdbool.h>
#include <stdint.h>

void gbcc_memory_increment(struct gbcc_core *gbc, uint16_t addr, bool override);
void gbcc_memory_copy(struct gbcc_core *gbc, uint16_t src, uint16_t dest, bool override);
void gbcc_memory_set_bit(struct gbcc_core *gbc, uint16_t addr, uint8_t b, bool override);
void gbcc_memory_clear_bit(struct gbcc_core *gbc, uint16_t addr, uint8_t b, bool override);

uint8_t gbcc_memory_read(struct gbcc_core *gbc, uint16_t addr, bool override);
void gbcc_memory_write(struct gbcc_core *gbc, uint16_t addr, uint8_t val, bool override);

void gbcc_link_cable_clock(struct gbcc_core *gbc);

#endif /* GBCC_MEMORY_H */
