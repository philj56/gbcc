#ifndef GBCC_PPU_H
#define GBCC_PPU_H
 
#include "gbcc.h"

void gbcc_ppu_clock(struct gbc *gbc);
void gbcc_disable_lcd(struct gbc *gbc);
void gbcc_enable_lcd(struct gbc *gbc);

#endif /* GBCC_PPU_H */
