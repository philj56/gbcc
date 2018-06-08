#ifndef GBCC_INPUT_H
#define GBCC_INPUT_H

#include "gbcc.h"
#include <SDL2/SDL.h>

void gbcc_input_process_all(struct gbc *gbc);
int gbcc_input_wait(void);

#endif /* GBCC_INPUT_H */
