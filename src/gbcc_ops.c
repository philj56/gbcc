#include <stdint.h>
#include <stdio.h>
#include "gbcc_ops.h"

uint8_t *CB_GET_OPERAND(struct gbc *gbc);

/* Main opcode jump table */
void (*gbcc_ops[0x100])(struct gbc *gbc) = {
/* 0x00 */	NOP,		LD_BC_d16,	LD_AT_BC_A,	INC_BC,
/* 0x04 */	INC_B,		DEC_B,		LD_B_d8,	RLCA,
/* 0x08 */	LD_a16_SP,	ADD_HL_BC,	LD_A_AT_BC,	DEC_BC,
/* 0x0C */	INC_C,		DEC_C,		LD_C_d8,	RRCA,
/* 0x10 */	STOP_0,		LD_DE_d16,	LD_AT_DE_A,	INC_DE,
/* 0x14 */	INC_D,		DEC_D,		LD_D_d8,	RLA,
/* 0x18 */	JR_r8,		ADD_HL_DE,	LD_A_AT_DE,	DEC_DE,
/* 0x1C */	INC_E,		DEC_E,		LD_E_d8,	RRA,
/* 0x20 */	JR_NZ_r8,	LD_AT_HL_d16,	LDI_AT_HL_A,	INC_HL,
/* 0x24 */	INC_H,		DEC_H,		LD_H_d8,	DAA,
/* 0x28 */	JR_Z_r8,	ADD_HL_HL,	LDI_A_AT_HL,	DEC_HL,
/* 0x2C */	INC_L,		DEC_L,		LD_L_d8,	CPL,
/* 0x30 */	JR_NC_r8,	LD_SP_d16,	LDD_AT_HL_A,	INC_SP,
/* 0x34 */	INC_AT_HL,	DEC_AT_HL,	LD_AT_HL_d8,	SCF,
/* 0x38 */	JR_C_r8,	ADD_HL_SP,	LDD_A_AT_HL,	DEC_SP,
/* 0x3C */	INC_A,		DEC_A,		LD_A_d8,	CCF,
/* 0x40 */	LD_B_B,		LD_B_C,		LD_B_D,		LD_B_E,
/* 0x44 */	LD_B_H,		LD_B_L,		LD_B_AT_HL,	LD_B_A,
/* 0x48 */	LD_C_B,		LD_C_C,		LD_C_D,		LD_C_E,
/* 0x4C */	LD_C_H,		LD_C_L,		LD_C_AT_HL,	LD_C_A,
/* 0x50 */	LD_D_B,		LD_D_C,		LD_D_D,		LD_D_E,
/* 0x54 */	LD_D_H,		LD_D_L,		LD_D_AT_HL,	LD_D_A,
/* 0x58 */	LD_E_B,		LD_E_C,		LD_E_D,		LD_E_E,
/* 0x5C */	LD_E_H,		LD_E_L,		LD_E_AT_HL,	LD_E_A,
/* 0x60 */	LD_H_B,		LD_H_C,		LD_H_D,		LD_H_E,
/* 0x64 */	LD_H_H,		LD_H_L,		LD_H_AT_HL,	LD_H_A,
/* 0x68 */	LD_L_B,		LD_L_C,		LD_L_D,		LD_L_E,
/* 0x6C */	LD_L_H,		LD_L_L,		LD_L_AT_HL,	LD_L_A,
/* 0x70 */	LD_AT_HL_B,	LD_AT_HL_C,	LD_AT_HL_D,	LD_AT_HL_E,
/* 0x74 */	LD_AT_HL_H,	LD_AT_HL_L,	HALT,		LD_AT_HL_A,
/* 0x78 */	LD_A_B,		LD_A_C,		LD_A_D,		LD_A_E,
/* 0x7C */	LD_A_H,		LD_A_L,		LD_A_AT_HL,	LD_A_A,
/* 0x80 */	ADD_A_B,	ADD_A_C,	ADD_A_D,	ADD_A_E,
/* 0x84 */	ADD_A_H,	ADD_A_L,	ADD_A_AT_HL,	ADD_A_A,
/* 0x88 */	ADC_A_B,	ADC_A_C,	ADC_A_D,	ADC_A_E,
/* 0x8C */	ADC_A_H,	ADC_A_L,	ADC_A_AT_HL,	ADC_A_A,
/* 0x90 */	SUB_B,		SUB_C,		SUB_D,		SUB_E,
/* 0x94 */	SUB_H,		SUB_L,		SUB_AT_HL,	SUB_A,
/* 0x98 */	SBC_A_B,	SBC_A_C,	SBC_A_D,	SBC_A_E,
/* 0x9C */	SBC_A_H,	SBC_A_L,	SBC_A_AT_HL,	SBC_A_A,
/* 0xA0 */	AND_B,		AND_C,		AND_D,		AND_E,
/* 0xA4 */	AND_H,		AND_L,		AND_AT_HL,	AND_A,
/* 0xA8 */	XOR_B,		XOR_C,		XOR_D,		XOR_E,
/* 0xAC */	XOR_H,		XOR_L,		XOR_AT_HL,	XOR_A,
/* 0xB0 */	OR_B,		OR_C,		OR_D,		OR_E,
/* 0xB4 */	OR_H,		OR_L,		OR_AT_HL,	OR_A,
/* 0xB8 */	CP_B,		CP_C,		CP_D,		CP_E,
/* 0xBC */	CP_H,		CP_L,		CP_AT_HL,	CP_A,
/* 0xC0 */	RET_NZ,		POP_BC,		JP_NZ_a16,	JP_a16,
/* 0xC4 */	CALL_NZ_a16,	PUSH_BC,	ADD_A_d8,	RST_00H,
/* 0xC8 */	RET_Z,		RET,		JP_Z_a16,	PREFIX_CB,
/* 0xCC */	CALL_Z_a16,	CALL_a16,	ADC_A_d8,	RST_08H,
/* 0xD0 */	RET_NC,		POP_DE,		JP_NC_a16,	INVALID,
/* 0xD4 */	CALL_NC_a16,	PUSH_DE,	SUB_d8,		RST_10H,
/* 0xD8 */	RET_C,		RETI,		JP_C_a16,	INVALID,
/* 0xDC */	CALL_C_a16,	INVALID,	SBC_A_d8,	RST_18H,
/* 0xE0 */	LDH_a8_A,	POP_HL,		LD_AT_C_A,	INVALID,
/* 0xE4 */	INVALID,	PUSH_HL,	AND_d8,		RST_20H,
/* 0xE8 */	ADD_SP_r8,	JP_HL,		LD_a16_A,	INVALID,
/* 0xEC */	INVALID,	INVALID,	XOR_d8,		RST_28H,
/* 0xF0 */	LDH_A_a8,	POP_AF,		LD_A_AT_C,	DI,
/* 0xF4 */	INVALID,	PUSH_AF,	OR_d8,		RST_30H,
/* 0xF8 */	LD_HL_SP_r8,	LD_SP_HL,	LD_A_a16,	EI,
/* 0xFC */	INVALID,	INVALID,	CP_d8,		RST_38H
};

/* CB-prefixed opcode jump table */
void (*gbcc_cb_ops[0x100])(struct gbc *gbc) = {
/* 0x00 */	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,
/* 0x04 */	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,
/* 0x08 */	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,
/* 0x0C */	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,
/* 0x10 */	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,
/* 0x14 */	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,
/* 0x18 */	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,
/* 0x1C */	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,
/* 0x20 */	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,
/* 0x24 */	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,
/* 0x28 */	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,
/* 0x2C */	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,
/* 0x30 */	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,
/* 0x34 */	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,
/* 0x38 */	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,
/* 0x3C */	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,	CB_SHIFT_OP,
/* 0x40 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,
/* 0x44 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,
/* 0x48 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,
/* 0x4C */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,
/* 0x50 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,
/* 0x54 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,
/* 0x58 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,
/* 0x5C */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,
/* 0x60 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,
/* 0x64 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,
/* 0x68 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,
/* 0x6C */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,
/* 0x70 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,
/* 0x74 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,
/* 0x78 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,
/* 0x7C */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,
/* 0x80 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0x84 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0x88 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0x8C */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0x90 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0x94 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0x98 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0x9C */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0xA0 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0xA4 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0xA8 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0xAC */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0xB0 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0xB4 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0xB8 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0xBC */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0xC0 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0xC4 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0xC8 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0xCC */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0xD0 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0xD4 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0xD8 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0xDC */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0xE0 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0xE4 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0xE8 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0xEC */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0xF0 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0xF4 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0xF8 */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	
/* 0xFC */	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP,	CB_BIT_OP
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
void LD_BC_d16(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_AT_BC_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void INC_BC(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void INC_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void DEC_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_B_d8(struct gbc *gbc)
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
void DEC_BC(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void INC_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void DEC_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_C_d8(struct gbc *gbc)
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
void LD_DE_d16(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_AT_DE_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void INC_DE(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void INC_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void DEC_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_D_d8(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RLA(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void JR_r8(struct gbc *gbc)
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
void DEC_DE(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void INC_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void DEC_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_E_d8(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RRA(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void JR_NZ_r8(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_AT_HL_d16(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LDI_AT_HL_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void INC_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void INC_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void DEC_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_H_d8(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void DAA(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void JR_Z_r8(struct gbc *gbc)
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
void DEC_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void INC_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void DEC_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_L_d8(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void CPL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void JR_NC_r8(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_SP_d16(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LDD_AT_HL_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void INC_SP(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void INC_AT_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void DEC_AT_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_AT_HL_d8(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SCF(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void JR_C_r8(struct gbc *gbc)
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
void DEC_SP(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void INC_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void DEC_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_A_d8(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void CCF(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_B_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_B_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_B_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_B_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_B_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_B_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_B_AT_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_B_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_C_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_C_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_C_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_C_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_C_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_C_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_C_AT_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_C_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_D_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_D_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_D_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_D_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_D_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_D_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_D_AT_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_D_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_E_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_E_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_E_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_E_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_E_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_E_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_E_AT_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_E_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_H_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_H_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_H_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_H_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_H_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_H_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_H_AT_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_H_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_L_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_L_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_L_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_L_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_L_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_L_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_L_AT_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_L_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_AT_HL_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_AT_HL_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_AT_HL_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_AT_HL_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_AT_HL_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_AT_HL_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void HALT(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_AT_HL_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_A_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_A_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_A_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_A_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_A_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_A_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_A_AT_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void LD_A_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void ADD_A_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void ADD_A_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void ADD_A_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void ADD_A_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void ADD_A_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void ADD_A_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void ADD_A_AT_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void ADD_A_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void ADC_A_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void ADC_A_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void ADC_A_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void ADC_A_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void ADC_A_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void ADC_A_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void ADC_A_AT_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void ADC_A_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SUB_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SUB_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SUB_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SUB_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SUB_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SUB_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SUB_AT_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SUB_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SBC_A_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SBC_A_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SBC_A_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SBC_A_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SBC_A_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SBC_A_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SBC_A_AT_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SBC_A_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void AND_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void AND_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void AND_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void AND_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void AND_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void AND_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void AND_AT_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void AND_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void XOR_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void XOR_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void XOR_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void XOR_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void XOR_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void XOR_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void XOR_AT_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void XOR_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void OR_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void OR_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void OR_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void OR_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void OR_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void OR_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void OR_AT_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void OR_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void CP_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void CP_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void CP_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void CP_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void CP_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void CP_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void CP_AT_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void CP_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RET_NZ(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void POP_BC(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void JP_NZ_a16(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void JP_a16(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void CALL_NZ_a16(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void PUSH_BC(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void ADD_A_d8(struct gbc *gbc)
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
void JP_Z_a16(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void PREFIX_CB(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void CALL_Z_a16(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void CALL_a16(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void ADC_A_d8(struct gbc *gbc)
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
void JP_NC_a16(struct gbc *gbc)
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
void SUB_d8(struct gbc *gbc)
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
void JP_C_a16(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void CALL_C_a16(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SBC_A_d8(struct gbc *gbc)
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
void AND_d8(struct gbc *gbc)
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
void XOR_d8(struct gbc *gbc)
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
void OR_d8(struct gbc *gbc)
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
void CP_d8(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RST_38H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RLC_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RLC_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RLC_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RLC_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RLC_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}

void RLC_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RLC_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RLC_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RRC_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RRC_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RRC_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RRC_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RRC_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RRC_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RRC_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RRC_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RL_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RL_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RL_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RL_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RL_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RL_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RL_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RL_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RR_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RR_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RR_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RR_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RR_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RR_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RR_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void RR_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SLA_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SLA_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SLA_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SLA_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SLA_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SLA_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SLA_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SLA_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SRA_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SRA_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SRA_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SRA_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SRA_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SRA_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SRA_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SRA_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SWAP_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SWAP_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SWAP_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SWAP_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SWAP_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SWAP_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SWAP_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SWAP_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SRL_B(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SRL_C(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SRL_D(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SRL_E(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SRL_H(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SRL_L(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SRL_HL(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}
void SRL_A(struct gbc *gbc)
{
	printf("Unimplemented opcode: 0x%02X\n", gbc->opcode);
}

void CB_SHIFT_OP(struct gbc *gbc)
{
	uint8_t *operand = CB_GET_OPERAND(gbc);
	uint8_t operation = gbc->opcode / 0x08u;
	uint8_t tmp;

	switch (operation) {
		case 0:	/* RLC */
			gbc->reg.cf = (*operand & 0x80u) >> 7;
			*operand = (*operand << 1) | (*operand >> 7);
			break;
		case 1:	/* RRC */
			gbc->reg.cf = *operand & 1;
			*operand = (*operand >> 1) | (*operand << 7);
			break;
		case 2:	/* RL */
			tmp = gbc->reg.cf;
			gbc->reg.cf = (*operand & 0x80u) >> 7;
			*operand = (*operand << 1) | tmp;
			break;
		case 3:	/* RR */
			tmp = gbc->reg.cf;
			gbc->reg.cf = *operand & 1;
			*operand = (*operand >> 1) | tmp << 7;
			break;
		case 4:	/* SLA */
			gbc->reg.cf = (*operand & 0x80u) >> 7;
			*operand = (*operand << 1);
			break;
		case 5:	/* SRA */
			gbc->reg.cf = *operand & 1;
			*operand = (*operand >> 1) | (*operand & 0x80u);
			break;
		case 6:	/* SWAP */
			gbc->reg.cf = 0;
			*operand = ((*operand & 0x0Fu) << 4) | ((*operand & 0xF0u) >> 4);
			break;
		case 7:	/* SRL */
			gbc->reg.cf = *operand & 1;
			*operand >>= 1;
			break;
	}
	gbc->reg.zf = (*operand == 0);
	gbc->reg.nf = 0;
	gbc->reg.hf = 0;
}

void CB_BIT_OP(struct gbc *gbc)
{
	uint8_t *operand = CB_GET_OPERAND(gbc);
	uint8_t mask = 1 << ((gbc->opcode & 0x0Fu) / 0x08u);
	uint8_t operation = ((gbc->opcode & 0xF0u) / 0x40u);

	switch (operation) {
		case 1: /* BIT */
			gbc->reg.zf = ~(*operand >> ((gbc->opcode & 0x0Fu) / 0x08u) & 1);
			gbc->reg.nf = 0;
			gbc->reg.hf = 1;
			break;
		case 2:	/* RES */
			*operand &= ~mask;
			break;
		case 3:	/* SET */
			*operand |= mask;
			break;
	}
}

uint8_t *CB_GET_OPERAND(struct gbc *gbc)
{
	switch (gbc->opcode % 0x08u) {
		case 0x0u:
			return &(gbc->reg.b);
		case 0x1u:
			return &(gbc->reg.c);
		case 0x2u:
			return &(gbc->reg.d);
		case 0x3u:
			return &(gbc->reg.e);
		case 0x4u:
			return &(gbc->reg.h);
		case 0x5u:
			return &(gbc->reg.l);
		case 0x6u:
			return &(gbc->memory[gbc->reg.hl]);
		case 0x7u:
			return &(gbc->reg.a);
		default:
			return NULL;
	}
}

