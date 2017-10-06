#include <stdint.h>
#include <stdio.h>
#include "gbcc_ops.h"
#include "gbcc_cpu.h"
#include "gbcc_memory.h"

uint8_t READ_OPERAND_MOD(struct gbc *gbc);
void WRITE_OPERAND_MOD(struct gbc *gbc, uint8_t val);
uint8_t READ_OPERAND_DIV(struct gbc *gbc, uint8_t offset);
void WRITE_OPERAND_DIV(struct gbc *gbc, uint8_t offset, uint8_t val);

/* Main opcode jump table */
void (*gbcc_ops[0x100])(struct gbc *gbc) = {
/* 0x00 */	NOP,		LD_d16,		LD_A,		INC_DEC_16_BIT,
/* 0x04 */	INC_DEC_8_BIT,	INC_DEC_8_BIT,	LD_d8,		SHIFT_A,
/* 0x08 */	LD_a16_SP,	ADD_HL,		LD_A,		INC_DEC_16_BIT,
/* 0x0C */	INC_DEC_8_BIT,	INC_DEC_8_BIT,	LD_d8,		SHIFT_A,
/* 0x10 */	STOP_0,		LD_d16,		LD_A,		INC_DEC_16_BIT,
/* 0x14 */	INC_DEC_8_BIT,	INC_DEC_8_BIT,	LD_d8,		SHIFT_A,
/* 0x18 */	JR,		ADD_HL,		LD_A,		INC_DEC_16_BIT,
/* 0x1C */	INC_DEC_8_BIT,	INC_DEC_8_BIT,	LD_d8,		SHIFT_A,
/* 0x20 */	JR_COND,	LD_d16,		LD_A,		INC_DEC_16_BIT,
/* 0x24 */	INC_DEC_8_BIT,	INC_DEC_8_BIT,	LD_d8,		DAA,
/* 0x28 */	JR_COND,	ADD_HL,		LD_A,		INC_DEC_16_BIT,
/* 0x2C */	INC_DEC_8_BIT,	INC_DEC_8_BIT,	LD_d8,		CPL,
/* 0x30 */	JR_COND,	LD_d16,		LD_A,		INC_DEC_16_BIT,
/* 0x34 */	INC_DEC_8_BIT,	INC_DEC_8_BIT,	LD_d8,		SCF,
/* 0x38 */	JR_COND,	ADD_HL,		LD_A,		INC_DEC_16_BIT,
/* 0x3C */	INC_DEC_8_BIT,	INC_DEC_8_BIT,	LD_d8,		CCF,
/* 0x40 */	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,
/* 0x44 */	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,
/* 0x48 */	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,
/* 0x4C */	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,
/* 0x50 */	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,
/* 0x54 */	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,
/* 0x58 */	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,
/* 0x5C */	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,
/* 0x60 */	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,
/* 0x64 */	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,
/* 0x68 */	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,
/* 0x6C */	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,
/* 0x70 */	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,
/* 0x74 */	LD_REG_REG,	LD_REG_REG,	HALT,		LD_REG_REG,
/* 0x78 */	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,
/* 0x7C */	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,	LD_REG_REG,
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
/* 0xC0 */	RET_COND,	PUSH_POP,	JP_COND,	JP,
/* 0xC4 */	CALL_COND,	PUSH_POP,	ALU_OP,		RST,
/* 0xC8 */	RET_COND,	RET,		JP_COND,	PREFIX_CB,
/* 0xCC */	CALL_COND,	CALL,		ALU_OP,		RST,
/* 0xD0 */	RET_COND,	PUSH_POP,	JP_COND,	INVALID,
/* 0xD4 */	CALL_COND,	PUSH_POP,	ALU_OP,		RST,
/* 0xD8 */	RET_COND,	RETI,		JP_COND,	INVALID,
/* 0xDC */	CALL_COND,	INVALID,	ALU_OP,		RST,
/* 0xE0 */	LDH_a8_A,	PUSH_POP,	LD_AT_C_A,	INVALID,
/* 0xE4 */	INVALID,	PUSH_POP,	ALU_OP,		RST,
/* 0xE8 */	ADD_SP_r8,	JP_HL,		LD_a16_A,	INVALID,
/* 0xEC */	INVALID,	INVALID,	ALU_OP,		RST,
/* 0xF0 */	LDH_A_a8,	PUSH_POP,	LD_A_AT_C,	DI,
/* 0xF4 */	INVALID,	PUSH_POP,	ALU_OP,		RST,
/* 0xF8 */	LD_HL_SP_r8,	LD_SP_HL,	LD_A_a16,	EI,
/* 0xFC */	INVALID,	INVALID,	ALU_OP,		RST
};

/*
 * Instruction lengths, in cycles. 0 means invalid instruction, or that the
 * duration of the instruction is conditional
 */
const uint8_t gbcc_op_times[0x100] = {
/* 0x00 */	4,  12, 8,  8,  4,  4,  8,  4,  20, 8,  8,  8,  4,  4,  8,  4,
/* 0x10 */	4,  12, 8,  8,  4,  4,  8,  4,  12, 8,  8,  8,  4,  4,  8,  4,
/* 0x20 */	0,  12, 8,  8,  4,  4,  8,  4,  0,  8,  8,  8,  4,  4,  8,  4,
/* 0x30 */	0,  12, 8,  8,  12, 12, 12, 4,  0,  8,  8,  8,  4,  4,  8,  4,
/* 0x40 */	4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/* 0x50 */	4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/* 0x60 */	4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/* 0x70 */	8,  8,  8,  8,  8,  8,  4,  8,  4,  4,  4,  4,  4,  4,  8,  4,
/* 0x80 */	4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/* 0x90 */	4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/* 0xA0 */	4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/* 0xB0 */	4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/* 0xC0 */	0,  12, 0,  16, 0,  16, 8,  16, 0,  16, 0,  4,  0,  24, 8,  16,
/* 0xD0 */	0,  12, 0,  0,  0,  16, 8,  16, 0,  16, 0,  0,  0,  0,  8,  16,
/* 0xE0 */	12, 12, 8,  0,  0,  16, 8,  16, 16, 4,  16, 0,  0,  0,  8,  16,
/* 0xF0 */	12, 12, 8,  4,  0,  16, 8,  16, 12, 8,  16, 4,  0,  0,  8,  16
};

/* Instruction sizes, in bytes. 0 means invalid instruction */
const uint8_t gbcc_op_sizes[0x100] = {
/* 0x00 */	1, 3, 1, 1, 1, 1, 2, 1, 3, 1, 1, 1, 1, 1, 2, 1,
/* 0x10 */	2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
/* 0x20 */	2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
/* 0x30 */	2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
/* 0x40 */	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 0x50 */	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 0x60 */	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 0x70 */	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 0x80 */	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 0x90 */	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 0xA0 */	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 0xB0 */	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 0xC0 */	1, 1, 3, 3, 3, 1, 2, 1, 1, 1, 3, 1, 3, 3, 2, 1,
/* 0xD0 */	1, 1, 3, 0, 3, 1, 2, 1, 1, 1, 3, 0, 3, 0, 2, 1,
/* 0xE0 */	2, 1, 2, 0, 0, 1, 2, 1, 2, 1, 3, 0, 0, 0, 2, 1,
/* 0xF0 */	2, 1, 2, 1, 0, 1, 2, 1, 2, 1, 3, 1, 0, 0, 2, 1
};

void INVALID(struct gbc *gbc)
{
	printf("Invalid opcode: 0x%02X\n", gbc->opcode);
}
void NOP(struct gbc *gbc)
{
	return;
}
void LD_a16_SP(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void STOP_0(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RST(struct gbc *gbc)
{
	uint8_t addr = gbc->opcode - 0xC7u;
	gbcc_memory_write(gbc, --gbc->reg.sp, (gbc->reg.pc & 0xFF00u) >> 8);
	gbcc_memory_write(gbc, --gbc->reg.sp, gbc->reg.pc & 0x00FFu);
	gbc->reg.pc = 0x0000u | addr;
}
void CALL(struct gbc *gbc)
{
	uint16_t addr = gbcc_fetch_instruction(gbc) | ((uint16_t)gbcc_fetch_instruction(gbc) << 8);
	gbcc_memory_write(gbc, --gbc->reg.sp, (gbc->reg.pc & 0xFF00u) >> 8);
	gbcc_memory_write(gbc, --gbc->reg.sp, gbc->reg.pc & 0x00FFu);
	gbc->reg.pc = addr;
}
void CALL_COND(struct gbc *gbc)
{
	uint16_t addr = gbcc_fetch_instruction(gbc) | ((uint16_t)gbcc_fetch_instruction(gbc) << 8);
	bool call = false;
	switch ((gbc->opcode - 0xC0u) / 0x08u) {
		case 0:	/* CALL NZ */
			if (!gbc->reg.zf) {
				call = true;
				gbcc_add_instruction_cycles(gbc, 24);
			} else {
				gbcc_add_instruction_cycles(gbc, 12);
			}
			break;
		case 1:	/* CALL Z */
			if (gbc->reg.zf) {
				call = true;
				gbcc_add_instruction_cycles(gbc, 24);
			} else {
				gbcc_add_instruction_cycles(gbc, 12);
			}
			break;
		case 2:	/* CALL NC */
			if (!gbc->reg.cf) {
				call = true;
				gbcc_add_instruction_cycles(gbc, 24);
			} else {
				gbcc_add_instruction_cycles(gbc, 12);
			}
			break;
		case 3:	/* CALL C */
			if (gbc->reg.cf) {
				call = true;
				gbcc_add_instruction_cycles(gbc, 24);
			} else {
				gbcc_add_instruction_cycles(gbc, 12);
			}
			break;
	}
	if (call) {
		gbcc_memory_write(gbc, --gbc->reg.sp, (gbc->reg.pc & 0xFF00u) >> 8);
		gbcc_memory_write(gbc, --gbc->reg.sp, gbc->reg.pc & 0x00FFu);
		gbc->reg.pc = addr;
	}
}
void RET(struct gbc *gbc) {
	gbc->reg.pc = gbcc_memory_read(gbc, gbc->reg.sp++);
	gbc->reg.pc |= (uint16_t)gbcc_memory_read(gbc, gbc->reg.sp++) << 8;
}
void RETI(struct gbc *gbc) {
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RET_COND(struct gbc *gbc) {
	bool ret = false;
	switch ((gbc->opcode - 0xC0u) / 0x08u) {
		case 0:	/* RET NZ */
			if (!gbc->reg.zf) {
				ret = true;
				gbcc_add_instruction_cycles(gbc, 20);
			} else {
				gbcc_add_instruction_cycles(gbc, 8);
			}
			break;
		case 1:	/* RET Z */
			if (gbc->reg.zf) {
				ret = true;
				gbcc_add_instruction_cycles(gbc, 20);
			} else {
				gbcc_add_instruction_cycles(gbc, 8);
			}
			break;
		case 2:	/* RET NC */
			if (!gbc->reg.cf) {
				ret = true;
				gbcc_add_instruction_cycles(gbc, 20);
			} else {
				gbcc_add_instruction_cycles(gbc, 8);
			}
			break;
		case 3:	/* RET C */
			if (gbc->reg.cf) {
				ret = true;
				gbcc_add_instruction_cycles(gbc, 20);
			} else {
				gbcc_add_instruction_cycles(gbc, 8);
			}
			break;
	}
	if (ret) {
		gbc->reg.pc = gbcc_memory_read(gbc, gbc->reg.sp++);
		gbc->reg.pc |= (uint16_t)gbcc_memory_read(gbc, gbc->reg.sp++) << 8;
	}
}
void DAA(struct gbc *gbc)
{
	uint8_t *op = &(gbc->reg.a);
	if ((*op & 0x0Fu) > 0x09u || gbc->reg.hf) {
		*op += 0x06u;
	}
	if ((*op & 0xF0u) > 0x90u || gbc->reg.cf) {
		*op += 0x60u;
		gbc->reg.cf = 1;
	} else {
		gbc->reg.cf = 0;
	}
	gbc->reg.zf = (*op == 0);
	gbc->reg.hf = 0;
}
void CPL(struct gbc *gbc)
{
	gbc->reg.a = ~gbc->reg.a;
	gbc->reg.nf = 1;
	gbc->reg.hf = 1;
}
void SCF(struct gbc *gbc)
{
	gbc->reg.cf = 1;
	gbc->reg.nf = 0;
	gbc->reg.hf = 0;
}
void CCF(struct gbc *gbc)
{
	gbc->reg.cf = ~gbc->reg.cf;
	gbc->reg.nf = 1;
	gbc->reg.hf = 1;
}
void LD_REG_REG(struct gbc *gbc)
{
	WRITE_OPERAND_DIV(gbc, 0x40u, READ_OPERAND_MOD(gbc));
}
void LD_d8(struct gbc *gbc)
{
	WRITE_OPERAND_DIV(gbc, 0x00u, gbcc_fetch_instruction(gbc));
}
void LD_d16(struct gbc *gbc)
{
	uint16_t val = gbcc_fetch_instruction(gbc) | ((uint16_t)gbcc_fetch_instruction(gbc) << 8);
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
	}
}
void LD_A(struct gbc *gbc)
{
	uint16_t op;
	switch (gbc->opcode / 0x10u) {
		case 0:
			op = gbc->reg.bc;
			break;
		case 1:
			op = gbc->reg.de;
			break;
		case 2:	/* LDI */
			op = gbc->reg.hl++;
			break;
		case 3:	/* LDD */
			op = gbc->reg.hl--;
			break;
	}
	switch((gbc->opcode % 0x10u) / 0x08u) {
		case 0:
			gbcc_memory_write(gbc, op, gbc->reg.a);
			break;
		case 1:
			gbc->reg.a = gbcc_memory_read(gbc, op);
			break;
	}
}
void PUSH_POP(struct gbc *gbc)
{
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
			op = &(gbc->reg.af);
			break;
	}
	switch (gbc->opcode / 0x04u) {
		case 0: /* POP */
			*op = gbcc_memory_read(gbc, gbc->reg.sp++);
			*op |= (uint16_t)gbcc_memory_read(gbc, gbc->reg.sp++) << 8;
			break;
		case 1: /* PUSH */
			gbcc_memory_write(gbc, --(gbc->reg.sp), *op >> 8);
			gbcc_memory_write(gbc, --(gbc->reg.sp), *op & 0x00FFu);
			break;
	}
}
void HALT(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void ALU_OP(struct gbc *gbc)
{
	uint8_t *op1 = &(gbc->reg.a);
	uint8_t op2;
	uint8_t tmp;
	uint8_t offset;
 	
	if (gbc->opcode < 0xC0u) {
		op2 = READ_OPERAND_MOD(gbc);
		offset = 0x80u;
	} else {
		op2 = gbcc_fetch_instruction(gbc);
		offset = 0xC0u;
	}
	
	switch ((gbc->opcode - offset) / 0x08u) {
		case 0: /* ADD */
			gbc->reg.hf = (((*op1 & 0x0Fu) + (op2 & 0x0Fu)) & 0x10u) == 0x10u;
			tmp = *op1;
			*op1 += op2;
			gbc->reg.zf = (*op1 == 0);
			gbc->reg.nf = 0;
			gbc->reg.cf = *op1 < tmp;
			break;
		case 1: /* ADC */
			gbc->reg.hf = (((*op1 & 0x0Fu) + (op2 & 0x0Fu) + gbc->reg.cf) & 0x10u) == 0x10u;
			tmp = *op1;
			*op1 += op2 + gbc->reg.cf;
			gbc->reg.zf = (*op1 == 0);
			gbc->reg.nf = 0;
			gbc->reg.cf = *op1 < tmp;
			break;
		case 2: /* SUB */
			gbc->reg.hf = ((*op1 & 0x0Fu) - (op2 & 0x0Fu)) > 0x0Fu;
			tmp = *op1;
			*op1 -= op2;
			gbc->reg.zf = (*op1 == 0);
			gbc->reg.nf = 1;
			gbc->reg.cf = *op1 > tmp;
			break;
		case 3: /* SBC */
			gbc->reg.hf = ((*op1 & 0x0Fu) - (op2 & 0x0Fu) - gbc->reg.cf) > 0x0Fu;
			tmp = *op1;
			*op1 -= op2 - gbc->reg.cf;
			gbc->reg.zf = (*op1 == 0);
			gbc->reg.nf = 1;
			gbc->reg.cf = *op1 > tmp;
			break;
		case 4: /* AND */
			*op1 &= op2;
			gbc->reg.zf = (*op1 == 0);
			gbc->reg.nf = 0;
			gbc->reg.hf = 1;
			gbc->reg.cf = 0;
			break;
		case 5: /* XOR */
			*op1 ^= op2;
			gbc->reg.zf = (*op1 == 0);
			gbc->reg.nf = 0;
			gbc->reg.hf = 0;
			gbc->reg.cf = 0;
			break;
		case 6: /* OR */
			*op1 |= op2;
			gbc->reg.zf = (*op1 == 0);
			gbc->reg.nf = 0;
			gbc->reg.hf = 0;
			gbc->reg.cf = 0;
			break;
		case 7: /* CP */
			gbc->reg.hf = ((*op1 & 0x0Fu) - (op2 & 0x0Fu)) > 0x0Fu;
			tmp = *op1 - op2;
			gbc->reg.zf = (tmp == 0);
			gbc->reg.nf = 1;
			gbc->reg.cf = tmp > *op1;
			break;
	}
}
void INC_DEC_8_BIT(struct gbc *gbc)
{
	uint8_t op = READ_OPERAND_DIV(gbc, 0x00u);
	switch ((gbc->opcode % 0x08u) / 0x05u) {
		case 0:	/* INC */
			gbc->reg.nf = 0;
			gbc->reg.hf = (((op & 0x0Fu) + 1) & 0x10u) == 0x10u;
			WRITE_OPERAND_DIV(gbc, 0x00u, ++op);
			break;
		case 1:	/* DEC */
			gbc->reg.nf = 1;
			gbc->reg.hf = (((op & 0x0Fu) - 1) & 0x10u) == 0x10u;
			WRITE_OPERAND_DIV(gbc, 0x00u, --op);
			break;
	}
	gbc->reg.zf = (op == 0);
}
void INC_DEC_16_BIT(struct gbc *gbc)
{
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
	}
	switch ((gbc->opcode % 0x10u) / 0x08u) {
		case 0:	/* INC */
			*op += 1;
			break;
		case 1:	/* DEC */
			*op -= 1;
			break;
	}
}
void ADD_HL(struct gbc *gbc)
{
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
	}
	gbc->reg.hf = (((op & 0x00FFu) + (gbc->reg.hl & 0x00FFu)) & 0x0100u) == 0x0100u;
	tmp = gbc->reg.hl;
	gbc->reg.hl += op;
	gbc->reg.cf = (tmp > gbc->reg.hl);
}
void SHIFT_A(struct gbc *gbc)
{
	uint8_t *op = &(gbc->reg.a);
	uint8_t operation = gbc->opcode / 0x08u;
	uint8_t tmp;

	switch (operation) {
		case 0:	/* RLC */
			gbc->reg.cf = (*op & 0x80u) >> 7;
			*op = (*op << 1) | (*op >> 7);
			break;
		case 1:	/* RRC */
			gbc->reg.cf = *op & 1;
			*op = (*op >> 1) | (*op << 7);
			break;
		case 2:	/* RL */
			tmp = gbc->reg.cf;
			gbc->reg.cf = (*op & 0x80u) >> 7;
			*op = (*op << 1) | tmp;
			break;
		case 3:	/* RR */
			tmp = gbc->reg.cf;
			gbc->reg.cf = *op & 1;
			*op = (*op >> 1) | tmp << 7;
			break;
	}
	gbc->reg.zf = 0;
	gbc->reg.nf = 0;
	gbc->reg.hf = 0;
}
void JP(struct gbc *gbc)
{
	gbc->reg.pc = (uint16_t)gbcc_fetch_instruction(gbc) 
		| (uint16_t)gbcc_fetch_instruction(gbc) << 8;
}
void JP_HL(struct gbc *gbc)
{
	gbc->reg.pc = gbc->reg.hl;
}
void JP_COND(struct gbc *gbc)
{
	uint16_t addr = (uint16_t)gbcc_fetch_instruction(gbc) 
		| (uint16_t)gbcc_fetch_instruction(gbc) << 8;
	switch ((gbc->opcode - 0xC0u) / 0x08u) {
		case 0:	/* JP NZ */
			if (!gbc->reg.zf) {
				gbc->reg.pc = addr;
				gbcc_add_instruction_cycles(gbc, 16);
			} else {
				gbcc_add_instruction_cycles(gbc, 12);
			}
			break;
		case 1:	/* JP Z */
			if (gbc->reg.zf) {
				gbc->reg.pc = addr;
				gbcc_add_instruction_cycles(gbc, 16);
			} else {
				gbcc_add_instruction_cycles(gbc, 12);
			}
			break;
		case 2:	/* JP NC */
			if (!gbc->reg.cf) {
				gbc->reg.pc = addr;
				gbcc_add_instruction_cycles(gbc, 16);
			} else {
				gbcc_add_instruction_cycles(gbc, 12);
			}
			break;
		case 3:	/* JP C */
			if (gbc->reg.cf) {
				gbc->reg.pc = addr;
				gbcc_add_instruction_cycles(gbc, 16);
			} else {
				gbcc_add_instruction_cycles(gbc, 12);
			}
			break;
	}
}
void JR(struct gbc *gbc)
{
	gbc->reg.pc += (int8_t)gbcc_fetch_instruction(gbc);
}
void JR_COND(struct gbc *gbc)
{
	uint16_t addr = gbc->reg.pc + (int8_t)gbcc_fetch_instruction(gbc); 
	switch ((gbc->opcode - 0x20u) / 0x08u) {
		case 0:	/* JR NZ */
			if (!gbc->reg.zf) {
				gbc->reg.pc = addr;
				gbcc_add_instruction_cycles(gbc, 12);
			} else {
				gbcc_add_instruction_cycles(gbc, 8);
			}
			break;
		case 1:	/* JR Z */
			if (gbc->reg.zf) {
				gbc->reg.pc = addr;
				gbcc_add_instruction_cycles(gbc, 12);
			} else {
				gbcc_add_instruction_cycles(gbc, 8);
			}
			break;
		case 2:	/* JR NC */
			if (!gbc->reg.cf) {
				gbc->reg.pc = addr;
				gbcc_add_instruction_cycles(gbc, 12);
			} else {
				gbcc_add_instruction_cycles(gbc, 8);
			}
			break;
		case 3:	/* JR C */
			if (gbc->reg.cf) {
				gbc->reg.pc = addr;
				gbcc_add_instruction_cycles(gbc, 12);
			} else {
				gbcc_add_instruction_cycles(gbc, 8);
			}
			break;
	}
}
void PREFIX_CB(struct gbc *gbc)
{
	if (gbc->opcode < 0x40u) {
		CB_SHIFT_OP(gbc);
	} else {
		CB_BIT_OP(gbc);
	}
}
void LDH_a8_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_AT_C_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void ADD_SP_r8(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_a16_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LDH_A_a8(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_A_AT_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void DI(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_HL_SP_r8(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_SP_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_A_a16(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void EI(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}


void CB_SHIFT_OP(struct gbc *gbc)
{
	uint8_t op = READ_OPERAND_MOD(gbc);
	uint8_t operation = gbc->opcode / 0x08u;
	uint8_t tmp;

	switch (operation) {
		case 0:	/* RLC */
			gbc->reg.cf = (op & 0x80u) >> 7;
			op = (op << 1) | (op >> 7);
			break;
		case 1:	/* RRC */
			gbc->reg.cf = op & 1;
			op = (op >> 1) | (op << 7);
			break;
		case 2:	/* RL */
			tmp = gbc->reg.cf;
			gbc->reg.cf = (op & 0x80u) >> 7;
			op = (op << 1) | tmp;
			break;
		case 3:	/* RR */
			tmp = gbc->reg.cf;
			gbc->reg.cf = op & 1;
			op = (op >> 1) | tmp << 7;
			break;
		case 4:	/* SLA */
			gbc->reg.cf = (op & 0x80u) >> 7;
			op = (op << 1);
			break;
		case 5:	/* SRA */
			gbc->reg.cf = op & 1;
			op = (op >> 1) | (op & 0x80u);
			break;
		case 6:	/* SWAP */
			gbc->reg.cf = 0;
			op = ((op & 0x0Fu) << 4) | ((op & 0xF0u) >> 4);
			break;
		case 7:	/* SRL */
			gbc->reg.cf = op & 1;
			op >>= 1;
			break;
	}
	gbc->reg.zf = (op == 0);
	gbc->reg.nf = 0;
	gbc->reg.hf = 0;

	WRITE_OPERAND_MOD(gbc, op);
	
	gbcc_add_instruction_cycles(gbc, 4);
	if ((gbc->opcode % 0x08u) == 6) {	/* Operating on (hl) */
		gbcc_add_instruction_cycles(gbc, 8);
	}
}

void CB_BIT_OP(struct gbc *gbc)
{
	uint8_t op = READ_OPERAND_MOD(gbc);
	uint8_t mask = 1 << ((gbc->opcode & 0x0Fu) / 0x08u);
	uint8_t operation = ((gbc->opcode & 0xF0u) / 0x40u);

	switch (operation) {
		case 1: /* BIT */
			gbc->reg.zf = ~(op >> ((gbc->opcode & 0x0Fu) / 0x08u) & 1);
			gbc->reg.nf = 0;
			gbc->reg.hf = 1;
			break;
		case 2:	/* RES */
			op &= ~mask;
			break;
		case 3:	/* SET */
			op |= mask;
			break;
	}

	WRITE_OPERAND_MOD(gbc, op);

	gbcc_add_instruction_cycles(gbc, 4);
	if ((gbc->opcode % 0x08u) == 6) {	/* Operating on (hl) */
		gbcc_add_instruction_cycles(gbc, 8);
	}
}

uint8_t READ_OPERAND_MOD(struct gbc *gbc)
{
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
			return gbcc_memory_read(gbc, gbc->reg.hl);
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
			gbcc_memory_write(gbc, gbc->reg.hl, val);
			break;
		case 7:
			gbc->reg.a = val;
			break;
	}
}

uint8_t READ_OPERAND_DIV(struct gbc *gbc, uint8_t offset)
{
	switch ((gbc->opcode - offset) / 0x08u) {
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
			return gbcc_memory_read(gbc, gbc->reg.hl);
		case 7:
			return gbc->reg.a;
		default:
			return 0;
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
			gbcc_memory_write(gbc, gbc->reg.hl, val);
			break;
		case 7:
			gbc->reg.a = val;
			break;
	}
}
