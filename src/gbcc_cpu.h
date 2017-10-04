#ifndef GBCC_CPU_H
#define GBCC_CPU_H

#include "gbcc.h"

extern void (*gbcc_ops[0x100])(struct gbc* gbc);
extern void (*gbcc_cb_ops[0x100])(struct gbc* gbc);
extern uint8_t gbcc_op_sizes[0x100];
extern uint8_t gbcc_op_times[0x100];

#endif /* GBCC_CPU_H */
