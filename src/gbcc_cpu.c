#include "gbcc.h"
#include "gbcc_bit_utils.h"
#include "gbcc_cpu.h"
#include "gbcc_debug.h"
#include "gbcc_memory.h"
#include "gbcc_ops.h"
#include <stdio.h>
#include <sys/time.h>


void gbcc_check_interrupts(struct gbc *gbc);
void gbcc_execute_instruction(struct gbc *gbc);

void gbcc_emulate_cycle(struct gbc *gbc)
{
	gbc->clock += 4;
	if (gbc->clock % 456 == 0) {
		printf("LY: %u\n", gbcc_memory_read(gbc, LY));
		gbc->memory.ioreg[LY - IOREG_START] += 1;
	}
	if ((gbc->clock % 456 == 4) && gbcc_memory_read(gbc, LY) == 144) { /* VBLANK */
		gbcc_memory_write(gbc, IF, gbcc_memory_read(gbc, IF) | 1u);
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
	uint8_t interrupt = (uint8_t)(int_enable & int_flags) & 0x1Fu;
	if (interrupt) {
		uint16_t addr;

		if (interrupt & (1u << 0u)) {
			addr = INT_VBLANK;
			printf("VBLANK\n");
			gbcc_memory_write(gbc, IF, int_flags & ~(1u << 0u));
		} else if (interrupt & (1u << 1u)) {
			addr = INT_LCDSTAT;
			printf("LCDSTAT\n");
			gbcc_memory_write(gbc, IF, int_flags & ~(1u << 1u));
		} else if (interrupt & (1u << 2u)) {
			addr = INT_TIMER;
			printf("TIMER\n");
			gbcc_memory_write(gbc, IF, int_flags & ~(1u << 2u));
		} else if (interrupt & (1u << 3u)) {
			addr = INT_SERIAL;
			printf("SERIAL\n");
			gbcc_memory_write(gbc, IF, int_flags & ~(1u << 3u));
		} else if (interrupt & (1u << 4u)) {
			addr = INT_JOYPAD;
			printf("JOYPAD\n");
			gbcc_memory_write(gbc, IF, int_flags & ~(1u << 4u));
		} else {
			addr = 0;
		}

		if (gbc->ime) {
			gbcc_add_instruction_cycles(gbc, 20);
			if (gbc->halt.set) {
				gbcc_add_instruction_cycles(gbc, 4);
			}
			gbcc_memory_write(gbc, --gbc->reg.sp, high_byte(gbc->reg.pc));
			gbcc_memory_write(gbc, --gbc->reg.sp, low_byte(gbc->reg.pc));
			gbc->reg.pc = addr;
		}
		gbc->halt.set = false;
		gbc->stop = false;
	}
}

void gbcc_execute_instruction(struct gbc *gbc)
{
	gbc->opcode = gbcc_fetch_instruction(gbc);
	gbcc_print_op(gbc);
	gbcc_ops[gbc->opcode](gbc);
	gbcc_print_registers(gbc);
	gbcc_add_instruction_cycles(gbc, gbcc_op_times[gbc->opcode]);
	/*if(gbc->reg.pc == 0x282a) {
		FILE *f = fopen("tile0.bin", "wb");
		fwrite(gbc->memory.emu_vram, VRAM_SIZE, 1, f);
		fclose(f);
		exit(0);
	}*/
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
