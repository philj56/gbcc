#include "gbcc.h"
#include "gbcc_bit_utils.h"
#include "gbcc_cpu.h"
#include "gbcc_debug.h"
#include "gbcc_memory.h"
#include "gbcc_ops.h"
#include "gbcc_video.h"
#include <stdio.h>
#include <sys/time.h>

/* TODO: Proper memory accesses with DMA */

static void gbcc_update_timers(struct gbc *gbc);
static void gbcc_check_interrupts(struct gbc *gbc);
static void gbcc_execute_instruction(struct gbc *gbc);

/* TODO: Check order of all of these */
void gbcc_emulate_cycle(struct gbc *gbc)
{
	gbc->clock += 4;
	if (gbc->dma.timer > 0) {
		gbc->dma.timer -= 4;
		if ((gbc->dma.source & 0xFFu) < OAM_SIZE) {
			uint8_t byte = gbcc_memory_read(gbc, gbc->dma.source, true);
			gbcc_memory_write(gbc, OAM_START + (gbc->dma.source & 0xFFu), byte, true);
			gbcc_log(GBCC_LOG_DEBUG, "DMA copied 0x%02X from 0x%04X to 0x%04X\n", byte, gbc->dma.source, OAM_START + (gbc->dma.source & 0xFFu));
			gbc->dma.source++;
		}
	//	gbcc_log(GBCC_LOG_DEBUG, "DMA in progress, %u\n", gbc->dma.timer);
	}
	gbcc_update_timers(gbc);
	//if (gbc->instruction_timer == 0) {
	gbcc_check_interrupts(gbc);
	gbcc_video_update(gbc);
	//}
	if (gbc->halt.set || gbc->stop) {
		return;
	}
	if (gbc->instruction_timer == 0) {
		gbcc_execute_instruction(gbc);
		//printf("0x%04X\n", gbc->reg.pc);
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
		uint64_t clock_div = 1u;
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
		if (!(gbc->clock % (0x400u / clock_div))) {
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
	uint8_t int_enable = gbc->memory.iereg;
	uint8_t int_flags = gbc->memory.ioreg[IF - IOREG_START];
	uint8_t interrupt = (uint8_t)(int_enable & int_flags) & 0x1Fu;
	if (interrupt && gbc->ime) {
		uint16_t addr;

		if (interrupt & (1u << 0u)) {
			addr = INT_VBLANK;
			gbcc_log(GBCC_LOG_DEBUG, "VBLANK interrupt\n");
			gbc->memory.ioreg[IF - IOREG_START] = int_flags & ~(1u << 0u);
		} else if (interrupt & (1u << 1u)) {
			addr = INT_LCDSTAT;
			gbcc_log(GBCC_LOG_DEBUG, "LCDSTAT interrupt\n");
			gbc->memory.ioreg[IF - IOREG_START] = int_flags & ~(1u << 1u);
		} else if (interrupt & (1u << 2u)) {
			addr = INT_TIMER;
			gbcc_log(GBCC_LOG_DEBUG, "TIMER interrupt\n");
			gbc->memory.ioreg[IF - IOREG_START] = int_flags & ~(1u << 2u);
		} else if (interrupt & (1u << 3u)) {
			addr = INT_SERIAL;
			gbcc_log(GBCC_LOG_DEBUG, "SERIAL interrupt\n");
			gbc->memory.ioreg[IF - IOREG_START] = int_flags & ~(1u << 3u);
		} else if (interrupt & (1u << 4u)) {
			addr = INT_JOYPAD;
			gbcc_log(GBCC_LOG_DEBUG, "JOYPAD interrupt\n");
			gbc->memory.ioreg[IF - IOREG_START] = int_flags & ~(1u << 4u);
		} else {
			gbcc_log(GBCC_LOG_ERROR, "False interrupt\n");
			addr = 0;
		}

		gbcc_add_instruction_cycles(gbc, 20);
		if (gbc->halt.set) {
			gbcc_add_instruction_cycles(gbc, 4);
		}
		gbcc_memory_write(gbc, --gbc->reg.sp, high_byte(gbc->reg.pc), false);
		gbcc_memory_write(gbc, --gbc->reg.sp, low_byte(gbc->reg.pc), false);
		gbc->reg.pc = addr;
		gbc->ime = false;
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
	uint8_t tmp = gbcc_memory_read(gbc, gbc->reg.pc, false);
	gbc->reg.pc += 1 - gbc->halt.skip;
	return tmp;
}

void gbcc_add_instruction_cycles(struct gbc *gbc, uint8_t cycles)
{
	gbc->instruction_timer += cycles;
}

