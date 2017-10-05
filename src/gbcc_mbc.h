#ifndef GBCC_MBC_H
#define GBCC_MBC_H

#include <stdint.h>
#include "gbcc.h"

uint8_t gbcc_mbc_none_read(struct gbc *gbc, uint16_t addr);
uint8_t gbcc_mbc_mbc1_read(struct gbc *gbc, uint16_t addr);
void gbcc_mbc_mbc1_write(struct gbc *gbc, uint16_t addr, uint8_t val);
uint8_t gbcc_mbc_mbc2_read(struct gbc *gbc, uint16_t addr);
void gbcc_mbc_mbc2_write(struct gbc *gbc, uint16_t addr, uint8_t val);
uint8_t gbcc_mbc_mbc3_read(struct gbc *gbc, uint16_t addr);
void gbcc_mbc_mbc3_write(struct gbc *gbc, uint16_t addr, uint8_t val);
uint8_t gbcc_mbc_mbc4_read(struct gbc *gbc, uint16_t addr);
void gbcc_mbc_mbc4_write(struct gbc *gbc, uint16_t addr, uint8_t val);
uint8_t gbcc_mbc_mbc5_read(struct gbc *gbc, uint16_t addr);
void gbcc_mbc_mbc5_write(struct gbc *gbc, uint16_t addr, uint8_t val);
uint8_t gbcc_mbc_mmm01_read(struct gbc *gbc, uint16_t addr);
void gbcc_mbc_mmm01_write(struct gbc *gbc, uint16_t addr, uint8_t val);

#endif /* GBCC_MBC_H */
