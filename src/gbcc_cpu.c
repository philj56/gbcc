#include <stdio.h>
#include <sys/time.h>
#include "gbcc.h"
#include "gbcc_cpu.h"
#include "gbcc_memory.h"
#include "gbcc_debug.h"

extern void (*gbcc_ops[0x100])(struct gbc* gbc);
extern uint8_t gbcc_op_sizes[0x100];
extern uint8_t gbcc_op_times[0x100];

void gbcc_check_interrupts(struct gbc *gbc);
void gbcc_execute_instruction(struct gbc *gbc);

void gbcc_emulate_cycle(struct gbc *gbc) 
{
	gbc->clock += 4;
	if (gbc->clock % 456 == 0) {
		//printf("LY: %u\n", gbcc_memory_read(gbc, LY));
		gbc->memory.ioreg[LY - IOREG_START] += 1;
	}
	if ((gbc->clock % 456 == 4) && gbcc_memory_read(gbc, LY) == 144) { /* VBLANK */
		gbcc_memory_write(gbc, IF, gbcc_memory_read(gbc, IF) | 1);
	} else if ((gbc->clock % 456 == 8) && gbcc_memory_read(gbc, LY) == 144) {
		gbcc_memory_write(gbc, IF, gbcc_memory_read(gbc, IF) & 0xFEu);
	} else if (gbcc_memory_read(gbc, LY) == 154 ) {
		gbc->memory.ioreg[LY - IOREG_START] = 0;
	}
	if (gbc->instruction_timer == 0) {
		gbcc_check_interrupts(gbc);
	}
	if (gbc->halt.set || gbc->stop) {
		return;
	}
	if (gbc->instruction_timer == 0) {
		gbcc_execute_instruction(gbc);
		//printf("pc: %04X\n", gbc->reg.pc);
	}
	gbc->instruction_timer -= 4;
}

void gbcc_check_interrupts(struct gbc *gbc)
{
	uint8_t int_enable = gbcc_memory_read(gbc, IE);
	uint8_t int_flags = gbcc_memory_read(gbc, IF);
	uint8_t interrupt = int_enable & int_flags & 0x1Fu;
	if (interrupt) {
		uint16_t addr;
		
		if (interrupt & (1 << 0)) {
			addr = INT_VBLANK;
			//printf("VBLANK\n");
			gbcc_memory_write(gbc, IF, int_flags & ~(1 << 0));
		} else if (interrupt & (1 << 1)) {
			addr = INT_LCDSTAT;
			printf("LCDSTAT\n");
			gbcc_memory_write(gbc, IF, int_flags & ~(1 << 1));
		} else if (interrupt & (1 << 2)) {
			addr = INT_TIMER;
			printf("TIMER\n");
			gbcc_memory_write(gbc, IF, int_flags & ~(1 << 2));
		} else if (interrupt & (1 << 3)) {
			addr = INT_SERIAL;
			printf("SERIAL\n");
			gbcc_memory_write(gbc, IF, int_flags & ~(1 << 3));
		} else if (interrupt & (1 << 4)) {
			addr = INT_JOYPAD;
			printf("JOYPAD\n");
			gbcc_memory_write(gbc, IF, int_flags & ~(1 << 4));
		} else {
			addr = 0;
		}

		if (gbc->ime) {
			gbcc_add_instruction_cycles(gbc, 20);
			if (gbc->halt.set) {
				gbcc_add_instruction_cycles(gbc, 4);
			}
			gbcc_memory_write(gbc, --gbc->reg.sp, (gbc->reg.pc & 0xFF00u) >> 8);
			gbcc_memory_write(gbc, --gbc->reg.sp, gbc->reg.pc & 0x00FFu);
			gbc->reg.pc = addr;
		}
		gbc->halt.set = false;
		gbc->stop = false;
	}
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
	gbcc_print_registers(gbc);
	gbcc_add_instruction_cycles(gbc, gbcc_op_times[gbc->opcode]);
	if(gbc->reg.pc == 0x282a) {
		FILE *f = fopen("tile0.bin", "wb");
		fwrite(gbc->memory.emu_vram, 16, 1, f);
		fclose(f);
		exit(0);
	}
	/*for (uint16_t i = 0; i < VRAM_SIZE; i++) {
		printf("%02X", gbc->memory.vram[i]);
	}*/
	printf("\n");
}

uint8_t gbcc_fetch_instruction(struct gbc *gbc)
{
	uint8_t tmp = gbcc_memory_read(gbc, gbc->reg.pc);
	gbc->reg.pc += 1 - gbc->halt.skip;
	gbc->reg.pc %= ROM0_SIZE + ROMX_SIZE;
	return tmp;
}

void gbcc_add_instruction_cycles(struct gbc *gbc, uint8_t cycles)
{
	gbc->instruction_timer += cycles;
}
