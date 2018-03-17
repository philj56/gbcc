#ifndef GBCC_DEBUG_H
#define GBCC_DEBUG_H

#include "gbcc.h"

extern const char* const op_dissassemblies[0x100];
void gbcc_print_registers(struct gbc *gbc);
void gbcc_print_op(struct gbc *gbc);

#endif /* GBCC_DEBUG_H */
