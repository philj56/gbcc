#include <stdio.h>
#include "gbcc.h"
#include "gbcc_cpu.h"

void gbcc_execute_instruction(struct gbc *gbc)
{
	gbc->opcode = gbc->cart.rom[gbc->reg.pc];
	for (uint8_t i = 0; i < gbcc_op_sizes[gbc->opcode]; i++) {
		printf("%02X", gbc->cart.rom[gbc->reg.pc++]);
	}
	printf("\n");
	gbcc_ops[gbc->opcode](gbc);
}
