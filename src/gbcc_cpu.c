#include "gbcc.h"
#include "gbcc_audio.h"
#include "gbcc_bit_utils.h"
#include "gbcc_cpu.h"
#include "gbcc_debug.h"
#include "gbcc_memory.h"
#include "gbcc_ops.h"
#include "gbcc_video.h"
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

static void gbcc_update_timers(struct gbc *gbc);
static void gbcc_check_interrupts(struct gbc *gbc);
static void gbcc_execute_instruction(struct gbc *gbc);

/* TODO: Check order of all of these */
void gbcc_emulate_cycle(struct gbc *gbc)
{
	gbc->clock += 4;
	if (gbc->dma.timer > 0) {
		gbc->dma.timer -= 4;
		if (low_byte(gbc->dma.source) < OAM_SIZE) {
			gbcc_memory_copy(gbc, gbc->dma.source, OAM_START + low_byte(gbc->dma.source), true);
			gbc->dma.source++;
		}
	}
	if (gbc->hdma.length > 0){
		/* FIXME: Only general-purpose hdma for now */
		uint16_t start = gbc->hdma.source;
		uint16_t end = gbc->hdma.source + gbc->hdma.length;
		uint16_t dest = gbc->hdma.dest;
		for (uint16_t src = start; src < end; src++, dest++) {
			gbcc_memory_copy(gbc, src, dest, true);
		}
		gbcc_memory_write(gbc, HDMA5, 0xFFu, true);
		printf("HDMA copied 0x%04X bytes from 0x%04X to 0x%04X\n", gbc->hdma.length, start, gbc->hdma.dest);
		gbc->hdma.length = 0;
	}
	gbcc_update_timers(gbc);
	//if (gbc->instruction_timer == 0) {
	gbcc_check_interrupts(gbc);
	gbcc_video_update(gbc);
	gbcc_audio_update(gbc);
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
	gbc->div_timer += 4u;
	if (gbc->div_timer == 64u) {
		gbcc_memory_increment(gbc, DIV, true);
		gbc->div_timer = 0;
	}
	if (check_bit(gbcc_memory_read(gbc, TAC, true), 2)) {
		uint8_t speed = gbcc_memory_read(gbc, TAC, true) & 0x03u;
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
			uint8_t tima = gbcc_memory_read(gbc, TIMA, true);
			uint8_t tmp = tima;
			tima += 1;
			gbcc_memory_increment(gbc, TIMA, true);
			if (tmp > tima) {
				gbcc_memory_copy(gbc, TMA, TIMA, true);
				gbcc_memory_set_bit(gbc, IF, 2, true);
				gbc->halt.set = false;
			}
		}
	}
}

void gbcc_check_interrupts(struct gbc *gbc)
{
	uint8_t int_enable = gbcc_memory_read(gbc, IE, true);
	uint8_t int_flags = gbcc_memory_read(gbc, IF, true);
	uint8_t interrupt = (uint8_t)(int_enable & int_flags) & 0x1Fu;
	if (interrupt && gbc->ime) {
		uint16_t addr;

		if (interrupt & bit(0)) {
			addr = INT_VBLANK;
			//gbcc_log(GBCC_LOG_DEBUG, "VBLANK interrupt\n");
			gbcc_memory_clear_bit(gbc, IF, 0, true);
		} else if (interrupt & bit(1)) {
			addr = INT_LCDSTAT;
			//gbcc_log(GBCC_LOG_DEBUG, "LCDSTAT interrupt\n");
			gbcc_memory_clear_bit(gbc, IF, 1, true);
		} else if (interrupt & bit(2)) {
			addr = INT_TIMER;
			//gbcc_log(GBCC_LOG_DEBUG, "TIMER interrupt\n");
			gbcc_memory_clear_bit(gbc, IF, 2, true);
		} else if (interrupt & bit(3)) {
			addr = INT_SERIAL;
			//gbcc_log(GBCC_LOG_DEBUG, "SERIAL interrupt\n");
			gbcc_memory_clear_bit(gbc, IF, 3, true);
		} else if (interrupt & bit(4)) {
			addr = INT_JOYPAD;
			//gbcc_log(GBCC_LOG_DEBUG, "JOYPAD interrupt\n");
			gbcc_memory_clear_bit(gbc, IF, 4, true);
		} else {
			gbcc_log(GBCC_LOG_ERROR, "False interrupt\n");
			addr = 0;
		}

		gbcc_add_instruction_cycles(gbc, 20);
		if (gbc->halt.set) {
			gbcc_add_instruction_cycles(gbc, 4);
		}
		gbcc_memory_write(gbc, --gbc->reg.sp, high_byte(gbc->reg.pc), true);
		gbcc_memory_write(gbc, --gbc->reg.sp, low_byte(gbc->reg.pc), true);
		gbc->reg.pc = addr;
		gbc->ime = false;
		gbc->halt.set = false;
		gbc->stop = false;
	}
}

void gbcc_execute_instruction(struct gbc *gbc)
{
	gbc->opcode = gbcc_fetch_instruction(gbc);
	//gbcc_print_op(gbc);
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

