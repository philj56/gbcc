#ifndef GBCC_SAVE_H
#define GBCC_SAVE_H

#include "gbcc.h"

void gbcc_save(struct gbc *gbc);
void gbcc_load(struct gbc *gbc);
void gbcc_save_state(struct gbc *gbc);
void gbcc_load_state(struct gbc *gbc);

#endif /* GBCC_SAVE_H */
