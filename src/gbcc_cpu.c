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

static void clock_div(struct gbc *gbc);
static void check_interrupts(struct gbc *gbc);
static void gbcc_cpu_clock(struct gbc *gbc);

/* TODO: Check order of all of these */
void gbcc_emulate_cycle(struct gbc *gbc)
{
	gbc->debug_clock++;
	gbc->clock++;
	gbc->clock &= 3u;
	check_interrupts(gbc);
	gbcc_apu_clock(gbc);
	gbcc_ppu_clock(gbc);
	gbcc_cpu_clock(gbc);
	clock_div(gbc);
	if (gbc->double_speed) {
		gbcc_cpu_clock(gbc);
		clock_div(gbc);
	}
}

void gbcc_cpu_clock(struct gbc *gbc)
{
	/* CPU clocks every 4 cycles */
	gbc->cpu_clock++;
	gbc->cpu_clock &= 3u;
	if (gbc->cpu_clock != 0) {
		return;
	}
	if (gbc->ime_timer.timer > 0) {
		gbc->ime_timer.timer--;
		if (gbc->ime_timer.timer == 1) {
			gbc->ime = gbc->ime_timer.target_state;
		}
	}
	if (gbc->dma.timer > 0) {
		gbc->dma.running = true;
		gbc->memory.oam[low_byte(gbc->dma.source)] = gbcc_memory_read(gbc, gbc->dma.source, false);
		gbc->dma.timer--;
		gbc->dma.source++;
	} else {
		gbc->dma.running = false;
	}
	if (gbc->dma.requested) {
		gbc->dma.requested = false;
		gbc->dma.timer = DMA_TIMER;
		gbc->dma.source = gbc->dma.new_source;
	}
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
		//printf("%04X\n", gbc->reg.pc);
		gbc->opcode = gbcc_fetch_instruction(gbc);
		//gbcc_print_op(gbc);
		gbc->instruction.running = true;
	}
	if (gbc->instruction.prefix_cb) {
		gbcc_ops[0xCB](gbc);
	} else {
		gbcc_ops[gbc->opcode](gbc);
	}
}

void clock_div(struct gbc *gbc)
{
	//printf("Div clock = %04X\n", gbc->div_timer);
	gbc->div_timer++;
	uint8_t tac = gbcc_memory_read(gbc, TAC, false);
	uint16_t mask;
	switch (tac & 0x03u) {
		/* 
		 * TIMA register detects the falling edge of a bit in
		 * the internal DIV timer, which is selected by TAC.
		 */
		case 0:
			mask = bit16(9);
			break;
		case 1:
			mask = bit16(3);
			break;
		case 2:
			mask = bit16(5);
			break;
		case 3:
			mask = bit16(7);
			break;
	}
	/* If TAC is disabled, this will always see 0 */
	mask *= check_bit(tac, 2);
	if (!(gbc->div_timer & mask) && gbc->tac_bit) {
		/* 
		 * The selected bit was previously high, and is now low, so
		 * the tima increment logic triggers.
		 */
		uint8_t tima = gbcc_memory_read(gbc, TIMA, false);
		tima++;
		if (tima == 0) {
			/* 
			 * TIMA overflow
			 * Rather than being reloaded immediately, TIMA takes
			 * 4 cycles to be reloaded, and another 4 to write, 
			 * so we just queue it here. This also affects the
			 * interrupt.
			 */
			gbc->tima_reload = 8;
		}
		gbcc_memory_write(gbc, TIMA, tima, false);
	}
	gbc->tac_bit = gbc->div_timer & mask;
	if (gbc->tima_reload > 0) {
		gbc->tima_reload--;
		if (gbc->tima_reload == 4) {
			/*
			 * Some more weird behaviour here: if TIMA has been
			 * written to while this copy & interrupt are waiting,
			 * they get cancelled, and everything proceeds as
			 * normal.
			 */
			if (gbcc_memory_read(gbc, TIMA, false) != 0) {
				gbc->tima_reload = 0;
			} else {
				gbcc_memory_copy(gbc, TMA, TIMA, false);
				gbcc_memory_set_bit(gbc, IF, 2, false);
			}
		}
		if (gbc->tima_reload == 0) {
			gbcc_memory_copy(gbc, TMA, TIMA, false);
		}
	}
	if (!gbc->apu.disabled) {
		/* APU also updates based on falling edge of DIV timer bit */
		if (gbc->double_speed) {
			mask = bit16(13);
		} else {
			mask = bit16(12);
		}
		if (!(gbc->div_timer & mask) && gbc->apu.div_bit) {
			gbcc_apu_sequencer_clock(gbc);
		}
		gbc->apu.div_bit = gbc->div_timer & mask;
	}
}

void check_interrupts(struct gbc *gbc)
{
	uint8_t iereg = gbcc_memory_read(gbc, IE, false);
	uint8_t ifreg = gbcc_memory_read(gbc, IF, false);
	uint8_t interrupt = (uint8_t)(iereg & ifreg) & 0x1Fu;
	if (interrupt) {
		gbc->halt.set = false;
		gbc->stop = false;
		if (!gbc->ime || gbc->interrupt.addr) {
			return;
		}
		/* 
		 * Interrupt priority is in memory address order; lower
		 * adresses have higher priority.
		 */
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
		}
	}
}

uint8_t gbcc_fetch_instruction(struct gbc *gbc)
{
	if (gbc->halt.skip) {
		/* HALT bug; CPU fails to increment pc */
		gbc->halt.skip = false;
		return gbcc_memory_read(gbc, gbc->reg.pc, false);
	} 
	return gbcc_memory_read(gbc, gbc->reg.pc++, false);
}
