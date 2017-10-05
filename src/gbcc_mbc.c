#include "gbcc_mbc.h"

uint8_t gbcc_mbc_none_read(struct gbc *gbc, uint16_t addr) {
	return gbc->cart.rom[addr];
}

/* TODO: IMPLEMENT!!! */
uint8_t gbcc_mbc_mbc1_read(struct gbc *gbc, uint16_t addr) {
	return 0;
}
void gbcc_mbc_mbc1_write(struct gbc *gbc, uint16_t addr, uint8_t val) {
}
uint8_t gbcc_mbc_mbc2_read(struct gbc *gbc, uint16_t addr) {
	return 0;
}
void gbcc_mbc_mbc2_write(struct gbc *gbc, uint16_t addr, uint8_t val) {
}
uint8_t gbcc_mbc_mbc3_read(struct gbc *gbc, uint16_t addr) {
	return 0;
}
void gbcc_mbc_mbc3_write(struct gbc *gbc, uint16_t addr, uint8_t val) {
}
uint8_t gbcc_mbc_mbc4_read(struct gbc *gbc, uint16_t addr) {
	return 0;
}
void gbcc_mbc_mbc4_write(struct gbc *gbc, uint16_t addr, uint8_t val) {
}
uint8_t gbcc_mbc_mbc5_read(struct gbc *gbc, uint16_t addr) {
	return 0;
}
void gbcc_mbc_mbc5_write(struct gbc *gbc, uint16_t addr, uint8_t val) {
}
uint8_t gbcc_mbc_mmm01_read(struct gbc *gbc, uint16_t addr) {
	return 0;
}
void gbcc_mbc_mmm01_write(struct gbc *gbc, uint16_t addr, uint8_t val) {
}
