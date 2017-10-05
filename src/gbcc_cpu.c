#include <stdio.h>
#include "gbcc.h"
#include "gbcc_cpu.h"

extern void (*gbcc_ops[0x100])(struct gbc* gbc);
extern uint8_t gbcc_op_sizes[0x100];
extern uint8_t gbcc_op_times[0x100];

uint8_t gbcc_fetch_instruction(struct gbc *gbc)
{
	return gbc->cart.rom[gbc->reg.pc++];
}

void gbcc_execute_instruction(struct gbc *gbc)
{
	gbc->opcode = gbcc_fetch_instruction(gbc);
	printf("%02X", gbc->opcode);
	for (uint8_t i = 0; i < gbcc_op_sizes[gbc->opcode] - 1; i++) {
		printf("%02X", gbc->cart.rom[gbc->reg.pc + i]);
	}
	printf("\n");
	gbcc_ops[gbc->opcode](gbc);
	gbcc_add_instruction_cycles(gbc, gbcc_op_times[gbc->opcode]);
}

void gbcc_add_instruction_cycles(struct gbc *gbc, uint8_t cycles)
{
	//TODO: Implement
}
