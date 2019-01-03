#include "gbcc.h"
#include "gbcc_apu.h"
#include "gbcc_bit_utils.h"
#include "gbcc_cpu.h"
#include "gbcc_debug.h"
#include "gbcc_hdma.h"
#include "gbcc_memory.h"
#include "gbcc_ops.h"
#include "gbcc_ppu.h"
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

static void update_timers(struct gbc *gbc);
static void check_interrupts(struct gbc *gbc);
static void gbcc_cpu_clock(struct gbc *gbc);

/* TODO: Check order of all of these */
void gbcc_emulate_cycle(struct gbc *gbc)
{
	if (gbc->double_speed) {
		gbcc_cpu_clock(gbc);
	}
	gbcc_cpu_clock(gbc);
	gbcc_ppu_clock(gbc);
	gbcc_apu_clock(gbc);
}

void gbcc_cpu_clock(struct gbc *gbc)
{
	gbc->clock += 4;
	if (gbc->dma.timer > 0) {
		gbc->dma.running = true;
		gbc->dma.timer--;
		if (low_byte(gbc->dma.source) < OAM_SIZE) {
			gbcc_memory_copy(gbc, gbc->dma.source, OAM_START + low_byte(gbc->dma.source), true);
			gbc->dma.source++;
		}
		if (gbc->dma.timer == 0) {
			gbc->dma.running = false;
		}
	}
	if (gbc->dma.requested) {
		gbc->dma.timer = DMA_TIMER;
		gbc->dma.requested = false;
		gbc->dma.source = gbc->dma.new_source;
	}
	update_timers(gbc);
	check_interrupts(gbc);
	if (!gbc->instruction.running) {
		if (gbc->interrupt.addr && gbc->ime) {
			INTERRUPT(gbc);
			return;
		}
		if (gbc->halt.set || gbc->stop) {
			return;
		}
		if (gbc->rst.request) {
			if (--gbc->rst.delay == 0) {
				gbc->rst.request = 0;
				gbc->reg.pc = gbc->rst.addr;
			}
		}
		gbc->opcode = gbcc_fetch_instruction(gbc);
		gbc->instruction.running = true;
	}
	if (gbc->instruction.prefix_cb) {
		gbcc_ops[0xCB](gbc);
	} else {
		gbcc_ops[gbc->opcode](gbc);
	}
}

void update_timers(struct gbc *gbc)
{
	gbc->div_timer++;
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
			default:
				gbcc_log_error("Impossible clock div\n");
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

void check_interrupts(struct gbc *gbc)
{
	uint8_t iereg = gbcc_memory_read(gbc, IE, false);
	uint8_t ifreg = gbcc_memory_read(gbc, IF, false);
	uint8_t interrupt = (uint8_t)(iereg & ifreg) & 0x1Fu;
	if (interrupt) {
		gbc->halt.set = false;
		if (!gbc->ime) {
			return;
		}
		if (check_bit(interrupt, 0)) {
			gbc->interrupt.addr = INT_VBLANK;
		} else if (check_bit(interrupt, 1)) {
			gbc->interrupt.addr = INT_LCDSTAT;
		} else if (check_bit(interrupt, 2)) {
			gbc->interrupt.addr = INT_TIMER;
		} else if (check_bit(interrupt, 3)) {
			gbc->interrupt.addr = INT_SERIAL;
		} else if (check_bit(interrupt, 4)) {
			gbc->interrupt.addr = INT_JOYPAD;
		} else {
			gbcc_log_error("False interrupt\n");
			gbc->interrupt.addr = 0;
		}
	}
}

uint8_t gbcc_fetch_instruction(struct gbc *gbc)
{
	if (gbc->halt.skip) {
		gbc->halt.skip = false;
		return gbcc_memory_read(gbc, gbc->reg.pc, false);
	} 
	return gbcc_memory_read(gbc, gbc->reg.pc++, false);
}
