#include <stdio.h>
#include "gbcc.h"
#include "gbcc_cpu.h"
#include "gbcc_memory.h"
#include "gbcc_debug.h"

extern void (*gbcc_ops[0x100])(struct gbc* gbc);
extern uint8_t gbcc_op_sizes[0x100];
extern uint8_t gbcc_op_times[0x100];

uint8_t gbcc_fetch_instruction(struct gbc *gbc)
{
	return gbcc_memory_read(gbc, gbc->reg.pc++);
}

void gbcc_execute_instruction(struct gbc *gbc)
{
	gbc->opcode = gbcc_fetch_instruction(gbc);
	printf("%02X", gbc->opcode);
	for (uint8_t i = 0; i < gbcc_op_sizes[gbc->opcode] - 1; i++) {
		printf("%02X", gbcc_memory_read(gbc, gbc->reg.pc + i));
	}
	printf("\t%s\n", op_dissassemblies[gbc->opcode]);
	gbcc_ops[gbc->opcode](gbc);
	gbcc_add_instruction_cycles(gbc, gbcc_op_times[gbc->opcode]);
	if(gbc->reg.pc >= 0x0297) {
	/*	FILE *f = fopen("tile0.bin", "wb");
		fwrite(gbc->memory.emu_vram, 16, 1, f);
		fclose(f);
		exit(0);*/
		gbcc_print_registers(gbc);
	printf("\n");
	}
}

void gbcc_add_instruction_cycles(struct gbc *gbc, uint8_t cycles)
{
	//TODO: Implement
}
