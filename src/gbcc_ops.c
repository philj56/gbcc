#include "gbcc.h"
#include "gbcc_bit_utils.h"
#include "gbcc_cpu.h"
#include "gbcc_debug.h"
#include "gbcc_memory.h"
#include "gbcc_ops.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define YIELD {gbc->instruction.step++; return;}

static uint8_t READ_OPERAND_MOD(struct gbc *gbc);
static void WRITE_OPERAND_MOD(struct gbc *gbc, uint8_t val);
static void WRITE_OPERAND_DIV(struct gbc *gbc, uint8_t offset, uint8_t val);
static bool mod_is_hl(struct gbc *gbc);
static bool div_is_hl(struct gbc *gbc, uint8_t offset);
static void done(struct gbc *gbc);

static uint8_t get_flag(struct gbc *gbc, uint8_t flag)
{
	return !!(gbc->reg.f & flag);
}

static void set_flag(struct gbc *gbc, uint8_t flag)
{
	gbc->reg.f |= flag;
}

static void clear_flag(struct gbc *gbc, uint8_t flag)
{
	gbc->reg.f &= (uint8_t)~flag;
}

static void toggle_flag(struct gbc *gbc, uint8_t flag)
{
	gbc->reg.f ^= flag;
}

static void cond_flag(struct gbc *gbc, uint8_t flag, bool cond) {
	if (cond) {
		set_flag(gbc, flag);
	} else {
		clear_flag(gbc, flag);
	}
}

/* Main opcode jump table */
void (*const gbcc_ops[0x100])(struct gbc *gbc) = {
/* 0x00 */	NOP,		LD_d16,		LD_A,		INC_DEC_16_BIT,
/* 0x04 */	INC_DEC_REG,	INC_DEC_REG,	LD_d8,		SHIFT_A,
/* 0x08 */	STORE_SP,	ADD_HL,		LD_A,		INC_DEC_16_BIT,
/* 0x0C */	INC_DEC_REG,	INC_DEC_REG,	LD_d8,		SHIFT_A,
/* 0x10 */	STOP,		LD_d16,		LD_A,		INC_DEC_16_BIT,
/* 0x14 */	INC_DEC_REG,	INC_DEC_REG,	LD_d8,		SHIFT_A,
/* 0x18 */	JR,		ADD_HL,		LD_A,		INC_DEC_16_BIT,
/* 0x1C */	INC_DEC_REG,	INC_DEC_REG,	LD_d8,		SHIFT_A,
/* 0x20 */	JR_COND,	LD_d16,		LD_A,		INC_DEC_16_BIT,
/* 0x24 */	INC_DEC_REG,	INC_DEC_REG,	LD_d8,		DAA,
/* 0x28 */	JR_COND,	ADD_HL,		LD_A,		INC_DEC_16_BIT,
/* 0x2C */	INC_DEC_REG,	INC_DEC_REG,	LD_d8,		CPL,
/* 0x30 */	JR_COND,	LD_d16,		LD_A,		INC_DEC_16_BIT,
/* 0x34 */	INC_DEC_HL,	INC_DEC_HL,	LD_d8,		SCF,
/* 0x38 */	JR_COND,	ADD_HL,		LD_A,		INC_DEC_16_BIT,
/* 0x3C */	INC_DEC_REG,	INC_DEC_REG,	LD_d8,		CCF,
/* 0x40 */	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,
/* 0x44 */	LD_REG_REG,	LD_REG_REG,	LD_REG_HL,	LD_REG_REG,
/* 0x48 */	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,
/* 0x4C */	LD_REG_REG,	LD_REG_REG,	LD_REG_HL,	LD_REG_REG,
/* 0x50 */	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,
/* 0x54 */	LD_REG_REG,	LD_REG_REG,	LD_REG_HL,	LD_REG_REG,
/* 0x58 */	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,
/* 0x5C */	LD_REG_REG,	LD_REG_REG,	LD_REG_HL,	LD_REG_REG,
/* 0x60 */	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,
/* 0x64 */	LD_REG_REG,	LD_REG_REG,	LD_REG_HL,	LD_REG_REG,
/* 0x68 */	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,
/* 0x6C */	LD_REG_REG,	LD_REG_REG,	LD_REG_HL,	LD_REG_REG,
/* 0x70 */	LD_REG_HL,	LD_REG_HL,	LD_REG_HL,	LD_REG_HL,
/* 0x74 */	LD_REG_HL,	LD_REG_HL,	HALT,		LD_REG_HL,
/* 0x78 */	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,
/* 0x7C */	LD_REG_REG,	LD_REG_REG,	LD_REG_HL,	LD_REG_REG,
/* 0x80 */	ALU_OP,		ALU_OP,		ALU_OP,		ALU_OP,
/* 0x84 */	ALU_OP,		ALU_OP,		ALU_OP,		ALU_OP,
/* 0x88 */	ALU_OP,		ALU_OP,		ALU_OP,		ALU_OP,
/* 0x8C */	ALU_OP,		ALU_OP,		ALU_OP,		ALU_OP,
/* 0x90 */	ALU_OP,		ALU_OP,		ALU_OP,		ALU_OP,
/* 0x94 */	ALU_OP,		ALU_OP,		ALU_OP,		ALU_OP,
/* 0x98 */	ALU_OP,		ALU_OP,		ALU_OP,		ALU_OP,
/* 0x9C */	ALU_OP,		ALU_OP,		ALU_OP,		ALU_OP,
/* 0xA0 */	ALU_OP,		ALU_OP,		ALU_OP,		ALU_OP,
/* 0xA4 */	ALU_OP,		ALU_OP,		ALU_OP,		ALU_OP,
/* 0xA8 */	ALU_OP,		ALU_OP,		ALU_OP,		ALU_OP,
/* 0xAC */	ALU_OP,		ALU_OP,		ALU_OP,		ALU_OP,
/* 0xB0 */	ALU_OP,		ALU_OP,		ALU_OP,		ALU_OP,
/* 0xB4 */	ALU_OP,		ALU_OP,		ALU_OP,		ALU_OP,
/* 0xB8 */	ALU_OP,		ALU_OP,		ALU_OP,		ALU_OP,
/* 0xBC */	ALU_OP,		ALU_OP,		ALU_OP,		ALU_OP,
/* 0xC0 */	RET_COND,	POP,		JP_COND,	JP,
/* 0xC4 */	CALL_COND,	PUSH,		ALU_OP,		RST,
/* 0xC8 */	RET_COND,	RET,		JP_COND,	PREFIX_CB,
/* 0xCC */	CALL_COND,	CALL,		ALU_OP,		RST,
/* 0xD0 */	RET_COND,	POP,		JP_COND,	INVALID,
/* 0xD4 */	CALL_COND,	PUSH,		ALU_OP,		RST,
/* 0xD8 */	RET_COND,	RETI,		JP_COND,	INVALID,
/* 0xDC */	CALL_COND,	INVALID,	ALU_OP,		RST,
/* 0xE0 */	LDH_a8,		POP,		LDH_C,		INVALID,
/* 0xE4 */	INVALID,	PUSH,		ALU_OP,		RST,
/* 0xE8 */	ADD_SP,		JP_HL,		LD_a16,		INVALID,
/* 0xEC */	INVALID,	INVALID,	ALU_OP,		RST,
/* 0xF0 */	LDH_a8,		POP,		LDH_C,		DI,
/* 0xF4 */	INVALID,	PUSH,		ALU_OP,		RST,
/* 0xF8 */	LD_HL_SP,	LD_SP_HL,	LD_a16,		EI,
/* 0xFC */	INVALID,	INVALID,	ALU_OP,		RST
};


void INTERRUPT(struct gbc *gbc)
{
	if (gbc->halt.no_interrupt) {
		if (gbc->instruction.step < 20) {
			YIELD;
		}
		gbc->halt.no_interrupt = false;
		gbc->halt.set = false;
		done(gbc);
		return;
	}
	uint8_t iereg = gbcc_memory_read(gbc, IE, false);
	uint8_t ifreg = gbcc_memory_read(gbc, IF, false);
	uint8_t interrupt = (uint8_t)(iereg & ifreg) & 0x1Fu;
	if (!interrupt && gbc->instruction.step < 4) {
		/* 
		 * Interrupt was cancelled by writing over a flag when pushing
		 * the high byte of pc.
		 */
		gbc->reg.pc = 0x0000u;
		gbc->ime = false;
		gbc->interrupt.addr = 0;
		done(gbc);
		return;
	}
	switch (gbc->instruction.step) {
		case 0:
			YIELD;
		case 1:
			YIELD;
		case 2:
			gbcc_memory_write(gbc, --gbc->reg.sp, high_byte(gbc->reg.pc), false);
			YIELD;
		case 3:
			gbcc_memory_write(gbc, --gbc->reg.sp, low_byte(gbc->reg.pc), false);
			YIELD;
		case 4:
			if (gbc->halt.set) {
				YIELD;
			}
	}
	switch (gbc->interrupt.addr) {
		case INT_VBLANK:
			gbcc_memory_clear_bit(gbc, IF, 0, false);
			break;
		case INT_LCDSTAT:
			gbcc_memory_clear_bit(gbc, IF, 1, false);
			break;
		case INT_TIMER:
			gbcc_memory_clear_bit(gbc, IF, 2, false);
			break;
		case INT_SERIAL:
			gbcc_memory_clear_bit(gbc, IF, 3, false);
			break;
		case INT_JOYPAD:
			gbcc_memory_clear_bit(gbc, IF, 4, false);
			break;
	}
	gbc->reg.pc = gbc->interrupt.addr;
	gbc->ime = false;
	gbc->interrupt.addr = 0;
	done(gbc);
}

/* Miscellaneous */

__attribute__((noreturn))
void INVALID(struct gbc *gbc)
{
	gbcc_log_error("Invalid opcode: 0x%02X\n", gbc->opcode);
	exit(EXIT_FAILURE);
}

void NOP(struct gbc *gbc)
{
	(void) gbc;
	done(gbc);
}

void STOP(struct gbc *gbc)
{
	uint8_t key1 = gbcc_memory_read(gbc, KEY1, true);
	if (gbc->mode == GBC && check_bit(key1, 0)) {
		key1 = clear_bit(key1, 0);
		gbc->double_speed = !gbc->double_speed;
		key1 = gbc->double_speed * bit(7);//toggle_bit(key1, 7);
		gbcc_memory_write(gbc, KEY1, key1, true);
	} else {
		gbc->stop = true;
	}
	gbcc_fetch_instruction(gbc); /* Discard next byte */
	done(gbc);
}

void HALT(struct gbc *gbc)
{
	uint8_t iereg = gbcc_memory_read(gbc, IE, false);
	uint8_t ifreg = gbcc_memory_read(gbc, IF, false);
	bool interrupt = (uint8_t)(iereg & ifreg) & 0x1Fu;
	if (gbc->ime) {
		/* HALT proceeds normally */
		gbc->halt.set = true;
		gbc->halt.no_interrupt = false;
		gbc->halt.skip = false;
	} else if (!interrupt) {
		/* HALT proceeds, but on resumption the interrupt isn't executed */
		gbc->halt.set = true;
		gbc->halt.no_interrupt = true;
		gbc->halt.skip = false;
	} else {
		/* HALT bug; next instruction doesn't increase PC */
		gbc->halt.set = false;
		gbc->halt.no_interrupt = false;
		gbc->halt.skip = true;
	}
	done(gbc);
}

void DAA(struct gbc *gbc)
{
	uint8_t *op = &(gbc->reg.a);
	if (get_flag(gbc, NF)) {
		if (get_flag(gbc, CF)) {
			*op -= 0x60u;
		}
		if (get_flag(gbc, HF)) {
			*op -= 0x06u;
		}
	} else {
		if (get_flag(gbc, CF) || *op > 0x99u) {
			*op += 0x60u;
			set_flag(gbc, CF);
		}
		if (get_flag(gbc, HF) || (*op & 0x0Fu) > 0x09u) {
			*op += 0x06u;
		}
	}
	cond_flag(gbc, ZF, (*op == 0));
	clear_flag(gbc, HF);
	done(gbc);
}

void CPL(struct gbc *gbc)
{
	gbc->reg.a = ~gbc->reg.a;
	set_flag(gbc, NF);
	set_flag(gbc, HF);
	done(gbc);
}

void SCF(struct gbc *gbc)
{
	set_flag(gbc, CF);
	clear_flag(gbc, NF);
	clear_flag(gbc, HF);
	done(gbc);
}

void CCF(struct gbc *gbc)
{
	toggle_flag(gbc, CF);
	clear_flag(gbc, NF);
	clear_flag(gbc, HF);
	done(gbc);
}

/* 
 * EI and DI take 1 M-cycle to take effect, so we don't immediately set the
 * IME here. However, EI immediately followed by DI will not result in an
 * interrupt, so we need to know when executing DI if the last instruction was
 * EI, hence setting the timer to 2, and making the IME change on step 1
 * normally.
 */
void EI(struct gbc *gbc)
{
	gbc->ime_timer.target_state = true;
	gbc->ime_timer.timer = 2;
	done(gbc);
}

void DI(struct gbc *gbc)
{
	if (gbc->ime_timer.timer > 0 && gbc->ime_timer.target_state == true) {
		gbc->ime = false;
		done(gbc);
		return;
	}
	gbc->ime_timer.target_state = false;
	gbc->ime_timer.timer = 2;
	done(gbc);
}

/* Loads */

void LD_REG_REG(struct gbc *gbc)
{
	WRITE_OPERAND_DIV(gbc, 0x40u, READ_OPERAND_MOD(gbc));
	done(gbc);
}

void LD_REG_HL(struct gbc *gbc)
{
	switch (gbc->instruction.step) {
		case 0:
			YIELD;
		case 1:
			gbc->instruction.op1 = READ_OPERAND_MOD(gbc);
			WRITE_OPERAND_DIV(gbc, 0x40u, gbc->instruction.op1);
	}
	done(gbc);
}

void LD_d8(struct gbc *gbc)
{
	switch (gbc->instruction.step) {
		case 0:
			YIELD;
		case 1:
			gbc->instruction.op1 = gbcc_fetch_instruction(gbc);
			if (div_is_hl(gbc, 0x00u)) {
				YIELD;
			}
		case 2:
			WRITE_OPERAND_DIV(gbc, 0x00u, gbc->instruction.op1);
	}
	done(gbc);
}

void LD_d16(struct gbc *gbc)
{
	switch (gbc->instruction.step) {
		case 0:
			YIELD;
		case 1:
			gbc->instruction.op1 = gbcc_fetch_instruction(gbc);
			YIELD;
		case 2:
			gbc->instruction.op2 = gbcc_fetch_instruction(gbc);
	}
	uint16_t val = cat_bytes(gbc->instruction.op1, gbc->instruction.op2);
	switch (gbc->opcode / 0x10u) {
		case 0:
			gbc->reg.bc = val;
			break;
		case 1:
			gbc->reg.de = val;
			break;
		case 2:
			gbc->reg.hl = val;
			break;
		case 3:
			gbc->reg.sp = val;
			break;
		default:
			gbcc_log_error("Impossible case in LD_d16\n");
			return;
	}
	done(gbc);
}

void LD_A(struct gbc *gbc)
{
	if (gbc->instruction.step == 0) {
		switch (gbc->opcode / 0x10u) {
			case 0:
				gbc->instruction.addr = gbc->reg.bc;
				break;
			case 1:
				gbc->instruction.addr = gbc->reg.de;
				break;
			case 2:	/* LDI */
				gbc->instruction.addr = gbc->reg.hl++;
				break;
			case 3:	/* LDD */
				gbc->instruction.addr = gbc->reg.hl--;
				break;
			default:
				gbcc_log_error("Impossible case in LD_A\n");
				return;
		}
		YIELD;
	}
	switch((gbc->opcode % 0x10u) / 0x08u) {
		case 0:
			gbcc_memory_write(gbc, gbc->instruction.addr, gbc->reg.a, false);
			break;
		case 1:
			gbc->reg.a = gbcc_memory_read(gbc, gbc->instruction.addr, false);
			break;
		default:
			gbcc_log_error("Impossible case in LD_A\n");
			return;
	}
	done(gbc);
}

void LD_a16(struct gbc *gbc)
{
	switch (gbc->instruction.step) {
		case 0:
			YIELD;
		case 1:
			gbc->instruction.op1 = gbcc_fetch_instruction(gbc);
			YIELD;
		case 2:
			gbc->instruction.op2 = gbcc_fetch_instruction(gbc);
			YIELD;
	}
	uint16_t addr = cat_bytes(gbc->instruction.op1, gbc->instruction.op2);
	switch ((gbc->opcode - 0xE0u) / 0x10u) {
		case 0:
			gbcc_memory_write(gbc, addr, gbc->reg.a, false);
			break;
		case 1:
			gbc->reg.a = gbcc_memory_read(gbc, addr, false);
			break;
		default:
			gbcc_log_error("Impossible case in LD_a16\n");
			break;
	}
	done(gbc);
}

void LDH_a8(struct gbc *gbc)
{
	switch (gbc->instruction.step) {
		case 0:
			YIELD;
		case 1:
			gbc->instruction.addr = 0xFF00u + gbcc_fetch_instruction(gbc);
			YIELD;
	}
	switch ((gbc->opcode - 0xE0u) / 0x10u) {
		case 0:
			gbcc_memory_write(gbc, gbc->instruction.addr, gbc->reg.a, false);
			break;
		case 1:
			gbc->reg.a = gbcc_memory_read(gbc, gbc->instruction.addr, false);
			break;
		default:
			gbcc_log_error("Impossible case in LD_OFFSET\n");
			return;
	}
	done(gbc);
}

void LDH_C(struct gbc *gbc)
{
	if (gbc->instruction.step == 0) {
		gbc->instruction.addr = 0xFF00u + gbc->reg.c;
		YIELD;
	}

	switch ((gbc->opcode - 0xE0u) / 0x10u) {
		case 0:
			gbcc_memory_write(gbc, gbc->instruction.addr, gbc->reg.a, false);
			break;
		case 1:
			gbc->reg.a = gbcc_memory_read(gbc, gbc->instruction.addr, false);
			break;
		default:
			gbcc_log_error("Impossible case in LD_OFFSET\n");
			return;
	}
	done(gbc);
}

void STORE_SP(struct gbc *gbc)
{
	switch (gbc->instruction.step) {
		case 0:
			YIELD;
		case 1:
			gbc->instruction.op1 = gbcc_fetch_instruction(gbc);
			YIELD;
		case 2:
			gbc->instruction.op2 = gbcc_fetch_instruction(gbc);
			YIELD;
		case 3:
			gbc->instruction.addr = cat_bytes(gbc->instruction.op1, gbc->instruction.op2);
			gbcc_memory_write(gbc, gbc->instruction.addr, low_byte(gbc->reg.sp), false);
			YIELD;
		case 4:
			gbcc_memory_write(gbc, gbc->instruction.addr+1, high_byte(gbc->reg.sp), false);
	}
	done(gbc);
}

void LD_HL_SP(struct gbc *gbc)
{
	switch (gbc->instruction.step) {
		case 0:
			YIELD;
		case 1:
			gbc->instruction.op1 = gbcc_fetch_instruction(gbc);
			YIELD;
	}
	uint16_t tmp = gbc->reg.sp;
	uint16_t tmp2 = (uint16_t)(gbc->reg.sp + (int8_t)gbc->instruction.op1);
	gbc->reg.hl = tmp2;
	cond_flag(gbc, HF, (tmp2 & 0x0Fu) < (tmp & 0x0Fu));
	cond_flag(gbc, CF, (tmp2 & 0xFFu) < (tmp & 0xFFu));
	clear_flag(gbc, ZF);
	clear_flag(gbc, NF);
	done(gbc);
}

void LD_SP_HL(struct gbc *gbc)
{
	if (gbc->instruction.step == 0) {
		YIELD;
	}
	gbc->reg.sp = gbc->reg.hl;
	done(gbc);
}

void POP(struct gbc *gbc)
{
	switch (gbc->instruction.step) {
		case 0:
			YIELD;
		case 1:
			gbc->instruction.op1 = gbcc_memory_read(gbc, gbc->reg.sp++, false);
			YIELD;
		case 2:
			gbc->instruction.op2 = gbcc_memory_read(gbc, gbc->reg.sp++, false);
	}
	uint16_t tmp = cat_bytes(gbc->instruction.op1, gbc->instruction.op2);
	switch ((gbc->opcode % 0x40u) / 0x10u) {
		case 0:
			gbc->reg.bc = tmp;
			break;
		case 1:
			gbc->reg.de = tmp;
			break;
		case 2:
			gbc->reg.hl = tmp;
			break;
		case 3:
			gbc->reg.af = tmp;
			/* Lower 4 bits of AF are always 0 */
			gbc->reg.af &= 0xFFF0u;
			break;
		default:
			gbcc_log_error("Impossible case in PUSH_POP\n");
			return;
	}
	done(gbc);
}

void PUSH(struct gbc *gbc)
{
	switch (gbc->instruction.step) {
		case 0:
			switch ((gbc->opcode % 0x40u) / 0x10u) {
				case 0:
					gbc->instruction.op1 = gbc->reg.b;
					gbc->instruction.op2 = gbc->reg.c;
					break;
				case 1:
					gbc->instruction.op1 = gbc->reg.d;
					gbc->instruction.op2 = gbc->reg.e;
					break;
				case 2:
					gbc->instruction.op1 = gbc->reg.h;
					gbc->instruction.op2 = gbc->reg.l;
					break;
				case 3:
					gbc->instruction.op1 = gbc->reg.a;
					gbc->instruction.op2 = gbc->reg.f;
					break;
				default:
					gbcc_log_error("Impossible case in PUSH_POP\n");
					return;
			}
			YIELD;
		case 1:
			YIELD;
		case 2:
			gbcc_memory_write(gbc, --(gbc->reg.sp), gbc->instruction.op1, false);
			YIELD;
		case 3:
			gbcc_memory_write(gbc, --(gbc->reg.sp), gbc->instruction.op2, false);
	}
	done(gbc);
}

/* ALU */

void ALU_OP(struct gbc *gbc)
{
	uint8_t *op1 = &(gbc->reg.a);
	uint8_t op2;
	uint8_t tmp;
	uint8_t offset;
	
	if (gbc->instruction.step == 0 && mod_is_hl(gbc)) {
		YIELD;
	}

	if (gbc->opcode < 0xC0u) {
		op2 = READ_OPERAND_MOD(gbc);
		offset = 0x80u;
	} else {
		op2 = gbcc_fetch_instruction(gbc);
		offset = 0xC0u;
	}

	switch ((gbc->opcode - offset) / 0x08u) {
		case 0: /* ADD */
			cond_flag(gbc, HF, (((*op1 & 0x0Fu) + (op2 & 0x0Fu)) & 0x10u) == 0x10u);
			tmp = *op1;
			*op1 += op2;
			cond_flag(gbc, ZF, (*op1 == 0));
			clear_flag(gbc, NF);
			cond_flag(gbc, CF, *op1 < tmp);
			break;
		case 1: /* ADC */
			cond_flag(gbc, HF, (((*op1 & 0x0Fu) + (op2 & 0x0Fu) + get_flag(gbc, CF)) & 0x10u) == 0x10u);
			*op1 += get_flag(gbc, CF);
			if (*op1 == 0 && get_flag(gbc, CF)) {
				set_flag(gbc, CF);
			} else {
				clear_flag(gbc, CF);
			}
			tmp = *op1;
			*op1 += op2;
			if (*op1 < tmp) {
				set_flag(gbc, CF);
			}
			cond_flag(gbc, ZF, (*op1 == 0));
			clear_flag(gbc, NF);
			break;
		case 2: /* SUB */
			cond_flag(gbc, HF, ((*op1 & 0x0Fu) - (op2 & 0x0Fu)) > 0x0Fu);
			tmp = *op1;
			*op1 -= op2;
			cond_flag(gbc, ZF, (*op1 == 0));
			set_flag(gbc, NF);
			cond_flag(gbc, CF, *op1 > tmp);
			break;
		case 3: /* SBC */
			cond_flag(gbc, HF, ((*op1 & 0x0Fu) - (op2 & 0x0Fu) - get_flag(gbc, CF)) > 0x0Fu);
			*op1 -= get_flag(gbc, CF);
			if (*op1 == 0xFF && get_flag(gbc, CF)) {
				set_flag(gbc, CF);
			} else {
				clear_flag(gbc, CF);
			}
			tmp = *op1;
			*op1 -= op2;
			if (*op1 > tmp) {
				set_flag(gbc, CF);
			}
			cond_flag(gbc, ZF, (*op1 == 0));
			set_flag(gbc, NF);
			break;
		case 4: /* AND */
			*op1 &= op2;
			cond_flag(gbc, ZF, (*op1 == 0));
			clear_flag(gbc, NF);
			set_flag(gbc, HF);
			clear_flag(gbc, CF);
			break;
		case 5: /* XOR */
			*op1 ^= op2;
			cond_flag(gbc, ZF, (*op1 == 0));
			clear_flag(gbc, NF);
			clear_flag(gbc, HF);
			clear_flag(gbc, CF);
			break;
		case 6: /* OR */
			*op1 |= op2;
			cond_flag(gbc, ZF, (*op1 == 0));
			clear_flag(gbc, NF);
			clear_flag(gbc, HF);
			clear_flag(gbc, CF);
			break;
		case 7: /* CP */
			cond_flag(gbc, HF, ((*op1 & 0x0Fu) - (op2 & 0x0Fu)) > 0x0Fu);
			tmp = *op1 - op2;
			cond_flag(gbc, ZF, (tmp == 0));
			set_flag(gbc, NF);
			cond_flag(gbc, CF, tmp > *op1);
			break;
		default:
			gbcc_log_error("Impossible case in ALU_OP\n");
			return;
	}
	done(gbc);
}

void INC_DEC_REG(struct gbc *gbc)
{
	uint8_t op;

	switch (gbc->opcode / 0x08u) {
		case 0:
			op = gbc->reg.b;
			break;
		case 1:
			op = gbc->reg.c;
			break;
		case 2:
			op = gbc->reg.d;
			break;
		case 3:
			op = gbc->reg.e;
			break;
		case 4:
			op = gbc->reg.h;
			break;
		case 5:
			op = gbc->reg.l;
			break;
		case 6:
			op = gbcc_memory_read(gbc, gbc->reg.hl, false);
			break;
		case 7:
			op = gbc->reg.a;
			break;
		default:
			gbcc_log_error("Impossible case in INC_DEC_8_BIT\n");
			return;
	}

	switch ((gbc->opcode % 0x08u) / 0x05u) {
		case 0:	/* INC */
			clear_flag(gbc, NF);
			cond_flag(gbc, HF, (((op & 0x0Fu) + 1) & 0x10u) == 0x10u);
			WRITE_OPERAND_DIV(gbc, 0x00u, ++op);
			break;
		case 1:	/* DEC */
			set_flag(gbc, NF);
			cond_flag(gbc, HF, (((op & 0x0Fu) - 1) & 0x10u) == 0x10u);
			WRITE_OPERAND_DIV(gbc, 0x00u, --op);
			break;
		default:
			gbcc_log_error("Impossible case in INC_DEC_8_BIT\n");
			return;
	}
	cond_flag(gbc, ZF, (op == 0));
	done(gbc);
}

void INC_DEC_HL(struct gbc *gbc)
{
	switch (gbc->instruction.step) {
		case 0:
			YIELD;
		case 1:
			gbc->instruction.op1 = gbcc_memory_read(gbc, gbc->reg.hl, false);
			YIELD;
	}
	uint8_t op = gbc->instruction.op1;
	switch ((gbc->opcode % 0x08u) / 0x05u) {
		case 0:	/* INC */
			clear_flag(gbc, NF);
			cond_flag(gbc, HF, (((op & 0x0Fu) + 1) & 0x10u) == 0x10u);
			WRITE_OPERAND_DIV(gbc, 0x00u, ++op);
			break;
		case 1:	/* DEC */
			set_flag(gbc, NF);
			cond_flag(gbc, HF, (((op & 0x0Fu) - 1) & 0x10u) == 0x10u);
			WRITE_OPERAND_DIV(gbc, 0x00u, --op);
			break;
		default:
			gbcc_log_error("Impossible case in INC_DEC_8_BIT\n");
			return;
	}
	cond_flag(gbc, ZF, (op == 0));
	done(gbc);
}

void INC_DEC_16_BIT(struct gbc *gbc)
{
	if (gbc->instruction.step == 0) {
		YIELD;
	}
	uint16_t *op;
	switch (gbc->opcode / 0x10u) {
		case 0:
			op = &(gbc->reg.bc);
			break;
		case 1:
			op = &(gbc->reg.de);
			break;
		case 2:
			op = &(gbc->reg.hl);
			break;
		case 3:
			op = &(gbc->reg.sp);
			break;
		default:
			gbcc_log_error("Impossible case in INC_DEC_16_BIT\n");
			return;
	}
	switch ((gbc->opcode % 0x10u) / 0x08u) {
		case 0:	/* INC */
			*op += 1u;
			break;
		case 1:	/* DEC */
			*op -= 1u;
			break;
		default:
			gbcc_log_error("Impossible case in INC_DEC_16_BIT\n");
			return;
	}
	done(gbc);
}

void ADD_HL(struct gbc *gbc)
{
	if (gbc->instruction.step == 0) {
		YIELD;
	}
	uint16_t op;
	uint16_t tmp;
	switch (gbc->opcode / 0x10u) {
		case 0:
			op = gbc->reg.bc;
			break;
		case 1:
			op = gbc->reg.de;
			break;
		case 2:
			op = gbc->reg.hl;
			break;
		case 3:
			op = gbc->reg.sp;
			break;
		default:
			gbcc_log_error("Impossible case in ADD_HL\n");
			return;
	}
	cond_flag(gbc, HF, ((gbc->reg.hl & 0x0FFFu) + (op & 0x0FFFu) & 0x1000u) == 0x1000u);
	tmp = gbc->reg.hl;
	gbc->reg.hl += op;
	cond_flag(gbc, CF, (tmp > gbc->reg.hl));
	clear_flag(gbc, NF);
	done(gbc);
}

void ADD_SP(struct gbc *gbc)
{
	switch (gbc->instruction.step) {
		case 0:
			YIELD;
		case 1:
			gbc->instruction.op1 = gbcc_fetch_instruction(gbc);
			YIELD;
		case 2:
			YIELD;
	}
	uint16_t tmp = gbc->reg.sp;
	gbc->reg.sp += (int8_t)gbc->instruction.op1;
	cond_flag(gbc, HF, (gbc->reg.sp & 0x0Fu) < (tmp & 0x0Fu));
	cond_flag(gbc, CF, (gbc->reg.sp & 0xFFu) < (tmp & 0xFFu));
	clear_flag(gbc, ZF);
	clear_flag(gbc, NF);
	done(gbc);
}

void SHIFT_A(struct gbc *gbc)
{
	uint8_t *op = &(gbc->reg.a);
	uint8_t tmp;

	switch (gbc->opcode / 0x08u) {
		case 0:	/* RLC */
			cond_flag(gbc, CF, (*op & 0x80u) >> 7u);
			*op = (uint8_t)(*op << 1u) | (uint8_t)(*op >> 7u);
			break;
		case 1:	/* RRC */
			cond_flag(gbc, CF, *op & 1u);
			*op = (uint8_t)(*op >> 1u) | (uint8_t)(*op << 7u);
			break;
		case 2:	/* RL */
			tmp = get_flag(gbc, CF);
			cond_flag(gbc, CF, (*op & 0x80u) >> 7u);
			*op = (uint8_t)(*op << 1u) | tmp;
			break;
		case 3:	/* RR */
			tmp = get_flag(gbc, CF);
			cond_flag(gbc, CF, *op & 1u);
			*op = (uint8_t)(*op >> 1u) | (uint8_t)(tmp << 7u);
			break;
		default:
			gbcc_log_error("Impossible case in SHIFT_A\n");
			return;
	}
	clear_flag(gbc, ZF);
	clear_flag(gbc, NF);
	clear_flag(gbc, HF);
	done(gbc);
}

/* Jumps */

void JP(struct gbc *gbc)
{
	switch (gbc->instruction.step) {
		case 0:
			YIELD;
		case 1:
			gbc->instruction.op1 = gbcc_fetch_instruction(gbc);
			YIELD;
		case 2:
			gbc->instruction.op2 = gbcc_fetch_instruction(gbc);
			YIELD;
	}
	gbc->reg.pc = cat_bytes(gbc->instruction.op1, gbc->instruction.op2);
	done(gbc);
}

void JP_HL(struct gbc *gbc)
{
	gbc->reg.pc = gbc->reg.hl;
	done(gbc);
}

void JP_COND(struct gbc *gbc)
{
	bool jp = false;
	switch (gbc->instruction.step) {
		case 0:
			YIELD;
		case 1:
			gbc->instruction.op1 = gbcc_fetch_instruction(gbc);
			YIELD;
		case 2:
			gbc->instruction.op2 = gbcc_fetch_instruction(gbc);
			gbc->instruction.addr = cat_bytes(gbc->instruction.op1, gbc->instruction.op2);
			switch ((gbc->opcode - 0xC0u) / 0x08u) {
				case 0:	/* JP NZ */
					jp = !get_flag(gbc, ZF);
					break;
				case 1:	/* JP Z */
					jp = get_flag(gbc, ZF);
					break;
				case 2:	/* JP NC */
					jp = !get_flag(gbc, CF);
					break;
				case 3:	/* JP C */
					jp = get_flag(gbc, CF);
					break;
			}
			if (jp) {
				gbc->reg.pc = gbc->instruction.addr;
				YIELD;
			} else {
				break;
			}
	}
	done(gbc);
}

void JR(struct gbc *gbc)
{
	switch (gbc->instruction.step) {
		case 0:
			YIELD;
		case 1:
			gbc->instruction.op1 = gbcc_fetch_instruction(gbc);
			YIELD;
		case 2:
			gbc->reg.pc += (int8_t)gbc->instruction.op1;
	}
	done(gbc);
}

void JR_COND(struct gbc *gbc)
{
	switch (gbc->instruction.step) {
		case 0:
			YIELD;
		case 1:
			gbc->instruction.op1 = gbcc_fetch_instruction(gbc);
			break;
		case 2: /* Jump was taken */
			done(gbc);
			return;
	}
	switch ((gbc->opcode - 0x20u) / 0x08u) {
		case 0:	/* JR NZ */
			if (!get_flag(gbc, ZF)) {
				gbc->reg.pc += (int8_t)gbc->instruction.op1;
				YIELD;
			}
			break;
		case 1:	/* JR Z */
			if (get_flag(gbc, ZF)) {
				gbc->reg.pc += (int8_t)gbc->instruction.op1;
				YIELD;
			}
			break;
		case 2:	/* JR NC */
			if (!get_flag(gbc, CF)) {
				gbc->reg.pc += (int8_t)gbc->instruction.op1;
				YIELD;
			}
			break;
		case 3:	/* JR C */
			if (get_flag(gbc, CF)) {
				gbc->reg.pc += (int8_t)gbc->instruction.op1;
				YIELD;
			}
			break;
		default:
			gbcc_log_error("Impossible case in JR_COND\n");
			return;
	}
	done(gbc);
}

/* Calls */

void CALL(struct gbc *gbc)
{
	switch (gbc->instruction.step) {
		case 0:
			YIELD;
		case 1:
			gbc->instruction.op1 = gbcc_fetch_instruction(gbc);
			YIELD;
		case 2:
			gbc->instruction.op2 = gbcc_fetch_instruction(gbc);
			YIELD;
		case 3:
			gbc->instruction.addr = cat_bytes(gbc->instruction.op1, gbc->instruction.op2);
			YIELD;
		case 4:
			gbcc_memory_write(gbc, --(gbc->reg.sp), high_byte(gbc->reg.pc), false);
			YIELD;
		case 5:
			gbcc_memory_write(gbc, --(gbc->reg.sp), low_byte(gbc->reg.pc), false);
	}
	gbc->reg.pc = gbc->instruction.addr;
	done(gbc);
}

void CALL_COND(struct gbc *gbc)
{
	switch (gbc->instruction.step) {
		case 0:
			YIELD;
		case 1:
			gbc->instruction.op1 = gbcc_fetch_instruction(gbc);
			YIELD;
		case 2:
			gbc->instruction.op2 = gbcc_fetch_instruction(gbc);
			switch ((gbc->opcode - 0xC0u) / 0x08u) {
				case 0:	/* CALL NZ */
					if (!get_flag(gbc, ZF)) {
						YIELD;
					}
					break;
				case 1:	/* CALL Z */
					if (get_flag(gbc, ZF)) {
						YIELD;
					}
					break;
				case 2:	/* CALL NC */
					if (!get_flag(gbc, CF)) {
						YIELD;
					}
					break;
				case 3:	/* CALL C */
					if (get_flag(gbc, CF)) {
						YIELD;
					}
					break;
			}
			break;
		case 3:
			gbc->instruction.addr = cat_bytes(gbc->instruction.op1, gbc->instruction.op2);
			YIELD;
		case 4:
			gbcc_memory_write(gbc, --(gbc->reg.sp), high_byte(gbc->reg.pc), false);
			YIELD;
		case 5:
			gbcc_memory_write(gbc, --(gbc->reg.sp), low_byte(gbc->reg.pc), false);
			gbc->reg.pc = gbc->instruction.addr;
	}
	done(gbc);
}

void RET(struct gbc *gbc)
{
	switch (gbc->instruction.step) {
		case 0:
			YIELD;
		case 1:
			gbc->instruction.op1 = gbcc_memory_read(gbc, gbc->reg.sp++, false);
			YIELD;
		case 2:
			gbc->instruction.op2 = gbcc_memory_read(gbc, gbc->reg.sp++, false);
			YIELD;
		case 3:
			gbc->reg.pc = cat_bytes(gbc->instruction.op1, gbc->instruction.op2);
	}
	done(gbc);
}

void RETI(struct gbc *gbc)
{
	RET(gbc);
	if (!gbc->instruction.running) {
		/* RET has finished */
		gbc->ime = true;
	}
}

void RET_COND(struct gbc *gbc)
{
	bool ret = false;
	switch (gbc->instruction.step) {
		case 0:
			YIELD;
		case 1:
			switch ((gbc->opcode - 0xC0u) / 0x08u) {
				case 0:	/* RET NZ */
					ret = !get_flag(gbc, ZF);
					break;
				case 1:	/* RET Z */
					ret = get_flag(gbc, ZF);
					break;
				case 2:	/* RET NC */
					ret = !get_flag(gbc, CF);
					break;
				case 3:	/* RET C */
					ret = get_flag(gbc, CF);
					break;
			}
			if (ret) {
				YIELD;
			} else {
				break;
			}
		case 2:
			gbc->instruction.op1 = gbcc_memory_read(gbc, gbc->reg.sp++, false);
			YIELD;
		case 3:
			gbc->instruction.op2 = gbcc_memory_read(gbc, gbc->reg.sp++, false);
			YIELD;
		case 4:
			gbc->reg.pc = cat_bytes(gbc->instruction.op1, gbc->instruction.op2);
	}
	done(gbc);
}

void RST(struct gbc *gbc)
{
	switch (gbc->instruction.step) {
		case 0:
			YIELD;
		case 1:
			YIELD;
		case 2:
			gbcc_memory_write(gbc, --gbc->reg.sp, high_byte(gbc->reg.pc), false);
			YIELD;
		case 3:
			gbcc_memory_write(gbc, --gbc->reg.sp, low_byte(gbc->reg.pc), false);
	}
	gbc->rst.addr = gbc->opcode - 0xC7u;
	gbc->rst.request = true;
	gbc->rst.delay = 1;
	done(gbc);
}

/* CB-prefix */

void PREFIX_CB(struct gbc *gbc)
{
	gbc->instruction.prefix_cb = true;
	switch (gbc->instruction.step) {
		case 0:
			YIELD;
		case 1:
			gbc->opcode = gbcc_fetch_instruction(gbc);
	}
	switch (gbc->opcode / 0x40u) {
		case 0:
			CB_SHIFT_OP(gbc);
			break;
		case 1:
			CB_BIT(gbc);
			break;
		case 2:
			CB_RES(gbc);
			break;
		case 3:
			CB_SET(gbc);
			break;
	}
}

void CB_SHIFT_OP(struct gbc *gbc)
{
	if (mod_is_hl(gbc)) {
		switch (gbc->instruction.step) {
			case 1:
				YIELD;
			case 2:
				gbc->instruction.op1 = READ_OPERAND_MOD(gbc);
				YIELD;
		}
	} else {
		gbc->instruction.op1 = READ_OPERAND_MOD(gbc);
	}
	uint8_t op = gbc->instruction.op1;
	uint8_t tmp;

	switch (gbc->opcode / 0x08u) {
		case 0:	/* RLC */
			cond_flag(gbc, CF, check_bit(op, 7));
			op = (uint8_t)(op << 1u) | (uint8_t)(op >> 7u);
			break;
		case 1:	/* RRC */
			cond_flag(gbc, CF, check_bit(op, 0));
			op = (uint8_t)(op >> 1u) | (uint8_t)(op << 7u);
			break;
		case 2:	/* RL */
			tmp = get_flag(gbc, CF);
			cond_flag(gbc, CF, check_bit(op, 7));
			op = (uint8_t)(op << 1u) | tmp;
			break;
		case 3:	/* RR */
			tmp = get_flag(gbc, CF);
			cond_flag(gbc, CF, check_bit(op, 0));
			op = (uint8_t)(op >> 1u) | (uint8_t)(tmp << 7u);
			break;
		case 4:	/* SLA */
			cond_flag(gbc, CF, check_bit(op, 7));
			op <<= 1u;
			break;
		case 5:	/* SRA */
			cond_flag(gbc, CF, check_bit(op, 0));
			op = (uint8_t)(op >> 1u) | (uint8_t)(op & 0x80u);
			break;
		case 6:	/* SWAP */
			clear_flag(gbc, CF);
			op = (uint8_t)((op & 0x0Fu) << 4u) | (uint8_t)((op & 0xF0u) >> 4u);
			break;
		case 7:	/* SRL */
			cond_flag(gbc, CF, check_bit(op, 0));
			op >>= 1u;
			break;
		default:
			gbcc_log_error("Impossible case in CB_SHIFT_OP\n");
			return;
	}
	cond_flag(gbc, ZF, (op == 0));
	clear_flag(gbc, NF);
	clear_flag(gbc, HF);

	WRITE_OPERAND_MOD(gbc, op);
	done(gbc);
}

void CB_BIT(struct gbc *gbc)
{
	if (gbc->instruction.step == 1 && mod_is_hl(gbc)) {
		YIELD;
	}
	uint8_t op = READ_OPERAND_MOD(gbc);
	uint8_t b = (gbc->opcode - 0x40u) / 0x08u;

	cond_flag(gbc, ZF, !check_bit(op, b));
	clear_flag(gbc, NF);
	set_flag(gbc, HF);
	done(gbc);
}

void CB_RES(struct gbc *gbc)
{
	if (mod_is_hl(gbc)) {
		switch (gbc->instruction.step) {
			case 1:
				YIELD;
			case 2:
				gbc->instruction.op1 = READ_OPERAND_MOD(gbc);
				YIELD;
		}
	} else {
		gbc->instruction.op1 = READ_OPERAND_MOD(gbc);
	}
	uint8_t b = (gbc->opcode - 0x80u) / 0x08u;

	WRITE_OPERAND_MOD(gbc, clear_bit(gbc->instruction.op1, b));
	done(gbc);
}

void CB_SET(struct gbc *gbc)
{
	if (mod_is_hl(gbc)) {
		switch (gbc->instruction.step) {
			case 1:
				YIELD;
			case 2:
				gbc->instruction.op1 = READ_OPERAND_MOD(gbc);
				YIELD;
		}
	} else {
		gbc->instruction.op1 = READ_OPERAND_MOD(gbc);
	}
	uint8_t b = (gbc->opcode - 0xC0u) / 0x08u;

	WRITE_OPERAND_MOD(gbc, set_bit(gbc->instruction.op1, b));
	done(gbc);
}

/* Helper functions */

uint8_t READ_OPERAND_MOD(struct gbc *gbc)
{
	uint8_t ret;
	switch (gbc->opcode % 0x08u) {
		case 0:
			return gbc->reg.b;
		case 1:
			return gbc->reg.c;
		case 2:
			return gbc->reg.d;
		case 3:
			return gbc->reg.e;
		case 4:
			return gbc->reg.h;
		case 5:
			return gbc->reg.l;
		case 6:
			ret = gbcc_memory_read(gbc, gbc->reg.hl, false);
			return ret;
		case 7:
			return gbc->reg.a;
		default:
			return 0;
	}
}

void WRITE_OPERAND_MOD(struct gbc *gbc, uint8_t val)
{
	switch (gbc->opcode % 0x08u) {
		case 0:
			gbc->reg.b = val;
			break;
		case 1:
			gbc->reg.c = val;
			break;
		case 2:
			gbc->reg.d = val;
			break;
		case 3:
			gbc->reg.e = val;
			break;
		case 4:
			gbc->reg.h = val;
			break;
		case 5:
			gbc->reg.l = val;
			break;
		case 6:
			gbcc_memory_write(gbc, gbc->reg.hl, val, false);
			break;
		case 7:
			gbc->reg.a = val;
			break;
	}
}

void WRITE_OPERAND_DIV(struct gbc *gbc, uint8_t offset, uint8_t val)
{
	switch ((gbc->opcode - offset) / 0x08u) {
		case 0:
			gbc->reg.b = val;
			break;
		case 1:
			gbc->reg.c = val;
			break;
		case 2:
			gbc->reg.d = val;
			break;
		case 3:
			gbc->reg.e = val;
			break;
		case 4:
			gbc->reg.h = val;
			break;
		case 5:
			gbc->reg.l = val;
			break;
		case 6:
			gbcc_memory_write(gbc, gbc->reg.hl, val, false);
			break;
		case 7:
			gbc->reg.a = val;
			break;
	}
}

bool mod_is_hl(struct gbc *gbc)
{
	return (gbc->opcode % 0x08u) == 6;
}

bool div_is_hl(struct gbc *gbc, uint8_t offset)
{
	return (gbc->opcode - offset) / 0x08u == 6;
}

void done(struct gbc *gbc)
{
	gbc->instruction.step = 0;
	gbc->instruction.running = false;
	gbc->instruction.prefix_cb = false;
}
