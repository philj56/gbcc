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

static void gbcc_update_timers(struct gbc *gbc);
static void gbcc_check_interrupts(struct gbc *gbc);
static void gbcc_execute_instruction(struct gbc *gbc);
static void gbcc_cpu_clock(struct gbc *gbc);

/* TODO: Check order of all of these */
void gbcc_emulate_cycle(struct gbc *gbc)
{
	gbcc_ppu_clock(gbc);
	gbcc_apu_clock(gbc);
	gbcc_cpu_clock(gbc);
	if (gbc->speed_mult == 2) {
		gbcc_cpu_clock(gbc);
	}
}

void gbcc_cpu_clock(struct gbc *gbc)
{
	gbc->clock += 4;
	if (gbc->dma.timer > 0) {
		gbc->dma.timer--;
		if (low_byte(gbc->dma.source) < OAM_SIZE) {
			gbcc_memory_copy(gbc, gbc->dma.source, OAM_START + low_byte(gbc->dma.source), true);
			gbc->dma.source++;
		}
	}
	if (gbc->dma.requested) {
		gbc->dma.timer = DMA_TIMER;
		gbc->dma.requested = false;
		gbc->dma.source = gbc->dma.new_source;
	}
	gbcc_update_timers(gbc);
	gbcc_check_interrupts(gbc);
	if (gbc->halt.set || gbc->stop) {
		return;
	}
	if (gbcc_op_fixed[gbc->opcode]) {
		gbcc_ops[gbc->opcode](gbc);
	}
	if (gbc->instruction_timer == 0) {
		gbcc_execute_instruction(gbc);
		if (gbc->rst.request) {
			if (--gbc->rst.delay == 0) {
				gbc->rst.request = 0;
				gbc->reg.pc = gbc->rst.addr;
			}
		}
	}
	gbc->instruction_timer--;
}

void gbcc_update_timers(struct gbc *gbc)
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

		if (check_bit(interrupt, 0)) {
			addr = INT_VBLANK;
			gbcc_memory_clear_bit(gbc, IF, 0, true);
		} else if (check_bit(interrupt, 1)) {
			addr = INT_LCDSTAT;
			gbcc_memory_clear_bit(gbc, IF, 1, true);
		} else if (check_bit(interrupt, 2)) {
			addr = INT_TIMER;
			gbcc_memory_clear_bit(gbc, IF, 2, true);
		} else if (check_bit(interrupt, 3)) {
			addr = INT_SERIAL;
			gbcc_memory_clear_bit(gbc, IF, 3, true);
		} else if (check_bit(interrupt, 4)) {
			addr = INT_JOYPAD;
			gbcc_memory_clear_bit(gbc, IF, 4, true);
		} else {
			gbcc_log(GBCC_LOG_ERROR, "False interrupt\n");
			addr = 0;
		}

		gbcc_add_instruction_cycles(gbc, 5);
		if (gbc->halt.set) {
			gbcc_add_instruction_cycles(gbc, 1);
		}
		/* 
		 * FIXME: EI instr. should take a full cycle to execute, but
		 * here it gets done instantly.
		 */
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
	/*printf("%04X\n", gbc->reg.pc);
	printf("LY %d\n", gbcc_memory_read(gbc, LY, true));
	printf("MODE %d\n", gbcc_memory_read(gbc, STAT, true) & 0x03u);
	*/
	//gbcc_print_op(gbc);
	gbcc_add_instruction_cycles(gbc, gbcc_op_times[gbc->opcode]);
	gbcc_ops[gbc->opcode](gbc);
	//gbcc_print_registers(gbc);
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

