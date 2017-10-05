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
/* 0x00 */	NOP,		LD_d16,		LD_AT_BC_A,	INC_DEC_16_BIT,
/* 0x04 */	INC_DEC_8_BIT,	INC_DEC_8_BIT,	LD_d8,		RLCA,
/* 0x08 */	LD_a16_SP,	ADD_HL_BC,	LD_A_AT_BC,	INC_DEC_16_BIT,
/* 0x0C */	INC_DEC_8_BIT,	INC_DEC_8_BIT,	LD_d8,		RRCA,
/* 0x10 */	STOP_0,		LD_d16,		LD_AT_DE_A,	INC_DEC_16_BIT,
/* 0x14 */	INC_DEC_8_BIT,	INC_DEC_8_BIT,	LD_d8,		RLA,
/* 0x18 */	JR,		ADD_HL_DE,	LD_A_AT_DE,	INC_DEC_16_BIT,
/* 0x1C */	INC_DEC_8_BIT,	INC_DEC_8_BIT,	LD_d8,		RRA,
/* 0x20 */	JR_COND,	LD_d16,		LDI_AT_HL_A,	INC_DEC_16_BIT,
/* 0x24 */	INC_DEC_8_BIT,	INC_DEC_8_BIT,	LD_d8,		DAA,
/* 0x28 */	JR_COND,	ADD_HL_HL,	LDI_A_AT_HL,	INC_DEC_16_BIT,
/* 0x2C */	INC_DEC_8_BIT,	INC_DEC_8_BIT,	LD_d8,		CPL,
/* 0x30 */	JR_COND,	LD_d16,		LDD_AT_HL_A,	INC_DEC_16_BIT,
/* 0x34 */	INC_DEC_8_BIT,	INC_DEC_8_BIT,	LD_d8,		SCF,
/* 0x38 */	JR_COND,	ADD_HL_SP,	LDD_A_AT_HL,	INC_DEC_16_BIT,
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
/* 0xC0 */	RET_NZ,		POP_BC,		JP_COND,	JP,
/* 0xC4 */	CALL_NZ_a16,	PUSH_BC,	ALU_OP,		RST_00H,
/* 0xC8 */	RET_Z,		RET,		JP_COND,	PREFIX_CB,
/* 0xCC */	CALL_Z_a16,	CALL_a16,	ALU_OP,		RST_08H,
/* 0xD0 */	RET_NC,		POP_DE,		JP_COND,	INVALID,
/* 0xD4 */	CALL_NC_a16,	PUSH_DE,	ALU_OP,		RST_10H,
/* 0xD8 */	RET_C,		RETI,		JP_COND,	INVALID,
/* 0xDC */	CALL_C_a16,	INVALID,	ALU_OP,		RST_18H,
/* 0xE0 */	LDH_a8_A,	POP_HL,		LD_AT_C_A,	INVALID,
/* 0xE4 */	INVALID,	PUSH_HL,	ALU_OP,		RST_20H,
/* 0xE8 */	ADD_SP_r8,	JP_HL,		LD_a16_A,	INVALID,
/* 0xEC */	INVALID,	INVALID,	ALU_OP,		RST_28H,
/* 0xF0 */	LDH_A_a8,	POP_AF,		LD_A_AT_C,	DI,
/* 0xF4 */	INVALID,	PUSH_AF,	ALU_OP,		RST_30H,
/* 0xF8 */	LD_HL_SP_r8,	LD_SP_HL,	LD_A_a16,	EI,
/* 0xFC */	INVALID,	INVALID,	ALU_OP,		RST_38H
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
void LD_AT_BC_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RLCA(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_a16_SP(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void ADD_HL_BC(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_A_AT_BC(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RRCA(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void STOP_0(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_AT_DE_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RLA(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void ADD_HL_DE(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_A_AT_DE(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RRA(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LDI_AT_HL_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void DAA(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void ADD_HL_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LDI_A_AT_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void CPL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LDD_AT_HL_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SCF(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void ADD_HL_SP(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LDD_A_AT_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void CCF(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
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
	uint16_t val = gbcc_fetch_instruction(gbc) | (gbcc_fetch_instruction(gbc) << 8);
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
	switch ((gbc->opcode % 0x08u) / 0x05u) {
		case 0:	/* INC */
			WRITE_OPERAND_DIV(gbc, 0x00u, READ_OPERAND_DIV(gbc, 0x00u) + 1);
			break;
		case 1:	/* DEC */
			WRITE_OPERAND_DIV(gbc, 0x00u, READ_OPERAND_DIV(gbc, 0x00u) - 1);
			break;
	}
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
void RET_NZ(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void POP_BC(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void JP(struct gbc *gbc)
{
	gbc->reg.pc = (uint16_t)gbcc_fetch_instruction(gbc) 
		| (uint16_t)gbcc_fetch_instruction(gbc) << 8;
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
void CALL_NZ_a16(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void PUSH_BC(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RST_00H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RET_Z(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RET(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void PREFIX_CB(struct gbc *gbc)
{
	if (gbc->opcode < 0x40u) {
		CB_SHIFT_OP(gbc);
	} else {
		CB_BIT_OP(gbc);
	}
}
void CALL_Z_a16(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void CALL_a16(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RST_08H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RET_NC(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void POP_DE(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void CALL_NC_a16(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void PUSH_DE(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RST_10H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RET_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RETI(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void CALL_C_a16(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RST_18H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LDH_a8_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void POP_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_AT_C_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void PUSH_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RST_20H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void ADD_SP_r8(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void JP_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_a16_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RST_28H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LDH_A_a8(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void POP_AF(struct gbc *gbc)
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
void PUSH_AF(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RST_30H(struct gbc *gbc)
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
void RST_38H(struct gbc *gbc)
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
		case 1:
			gbc->reg.c = val;
		case 2:
			gbc->reg.d = val;
		case 3:
			gbc->reg.e = val;
		case 4:
			gbc->reg.h = val;
		case 5:
			gbc->reg.l = val;
		case 6:
			gbcc_memory_write(gbc, gbc->reg.hl, val);
		case 7:
			gbc->reg.a = val;
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
		case 1:
			gbc->reg.c = val;
		case 2:
			gbc->reg.d = val;
		case 3:
			gbc->reg.e = val;
		case 4:
			gbc->reg.h = val;
		case 5:
			gbc->reg.l = val;
		case 6:
			gbcc_memory_write(gbc, gbc->reg.hl, val);
		case 7:
			gbc->reg.a = val;
	}
}
