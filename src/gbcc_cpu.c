#include "gbcc.h"
#include "gbcc_bit_utils.h"
#include "gbcc_cpu.h"
#include "gbcc_debug.h"
#include "gbcc_memory.h"
#include "gbcc_ops.h"
#include "gbcc_video.h"
#include <stdio.h>
#include <sys/time.h>

static void gbcc_update_timers(struct gbc *gbc);
static void gbcc_check_interrupts(struct gbc *gbc);
static void gbcc_execute_instruction(struct gbc *gbc);

/* TODO: Check order of all of these */
void gbcc_emulate_cycle(struct gbc *gbc)
{
	gbc->clock += 4;
	if (gbc->dma.timer > 0) {
		gbcc_log(GBCC_LOG_DEBUG, "DMA in progress, %u\n", gbc->dma.timer);
		gbc->dma.timer -= 4;
	}
	gbcc_update_timers(gbc);
	gbcc_video_update(gbc);
	//if (gbc->instruction_timer == 0) {
		gbcc_check_interrupts(gbc);
	//}
	if (gbc->halt.set || gbc->stop) {
		return;
	}
	if (gbc->instruction_timer == 0) {
		gbcc_execute_instruction(gbc);
		//gbcc_log(GBCC_LOG_DEBUG, "pc: %04X\n", gbc->reg.pc);
	}
	gbc->instruction_timer -= 4;
}

void gbcc_update_timers(struct gbc *gbc)
{
	if (!(gbc->clock % DIV_INC_CLOCKS)) {
		gbc->memory.ioreg[DIV - IOREG_START] += 1;
	}
	if (gbc->memory.ioreg[TAC - IOREG_START] & 0x04u) {
		uint8_t speed = gbc->memory.ioreg[TAC - IOREG_START] & 0x03u;
		uint8_t clock_div = 1u;
		switch (speed) {
			case 0:
				clock_div = 1u;
				break;
			case 1:
				clock_div = 64u;
				break;
			case 2:
				clock_div = 16u;
				break;
			case 3:
				clock_div = 4u;
				break;
		}
		if (!(gbc->clock % (0x400 / clock_div))) {
			uint8_t *tima = &(gbc->memory.ioreg[TIMA - IOREG_START]);
			uint8_t tmp = *tima;
			*tima += 1;
			if (tmp > *tima) {
				*tima = gbc->memory.ioreg[TMA - IOREG_START];
				gbc->memory.ioreg[IF - IOREG_START] |= 0x04u;
				gbc->halt.set = false;
			}

		}
	}
}

void gbcc_check_interrupts(struct gbc *gbc)
{
	uint8_t int_enable = gbcc_memory_read(gbc, IE);
	uint8_t int_flags = gbcc_memory_read(gbc, IF);
	uint8_t interrupt = (uint8_t)(int_enable & int_flags) & 0x1Fu;
	if (interrupt && gbc->ime) {
		uint16_t addr;

		if (interrupt & (1u << 0u)) {
			addr = INT_VBLANK;
			gbcc_log(GBCC_LOG_DEBUG, "VBLANK\n");
			gbcc_memory_write(gbc, IF, int_flags & ~(1u << 0u));
		} else if (interrupt & (1u << 1u)) {
			addr = INT_LCDSTAT;
			gbcc_log(GBCC_LOG_DEBUG, "LCDSTAT\n");
			gbcc_memory_write(gbc, IF, int_flags & ~(1u << 1u));
		} else if (interrupt & (1u << 2u)) {
			addr = INT_TIMER;
			gbcc_log(GBCC_LOG_DEBUG, "TIMER\n");
			gbcc_memory_write(gbc, IF, int_flags & ~(1u << 2u));
		} else if (interrupt & (1u << 3u)) {
			addr = INT_SERIAL;
			gbcc_log(GBCC_LOG_DEBUG, "SERIAL\n");
			gbcc_memory_write(gbc, IF, int_flags & ~(1u << 3u));
		} else if (interrupt & (1u << 4u)) {
			addr = INT_JOYPAD;
			gbcc_log(GBCC_LOG_DEBUG, "JOYPAD\n");
			gbcc_memory_write(gbc, IF, int_flags & ~(1u << 4u));
		} else {
			addr = 0;
		}

		//if (gbc->ime) {
			gbcc_add_instruction_cycles(gbc, 20);
			if (gbc->halt.set) {
				gbcc_add_instruction_cycles(gbc, 4);
			}
			gbcc_memory_write(gbc, --gbc->reg.sp, high_byte(gbc->reg.pc));
			gbcc_memory_write(gbc, --gbc->reg.sp, low_byte(gbc->reg.pc));
			gbc->reg.pc = addr;
		//}
		gbc->halt.set = false;
		gbc->stop = false;
	}
}

void gbcc_execute_instruction(struct gbc *gbc)
{
	gbc->opcode = gbcc_fetch_instruction(gbc);
	gbcc_print_op(gbc);
	gbcc_ops[gbc->opcode](gbc);
	gbcc_add_instruction_cycles(gbc, gbcc_op_times[gbc->opcode]);
}

uint8_t gbcc_fetch_instruction(struct gbc *gbc)
{
	uint8_t tmp = gbcc_memory_read(gbc, gbc->reg.pc);
	gbc->reg.pc += 1 - gbc->halt.skip;
	return tmp;
}

void gbcc_add_instruction_cycles(struct gbc *gbc, uint8_t cycles)
{
	gbc->instruction_timer += cycles;
}

