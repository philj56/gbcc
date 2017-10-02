#include <stdint.h>
#include <stdio.h>
#include "gbcc_ops.h"

/* Main opcode jump table */
void (*gbcc_ops[0x100])(void) = {
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
void (*gbcc_cb_ops[0x100])(void) = {
/* 0x00 */	RLC_B,		RLC_C,		RLC_D,		RLC_E,
/* 0x04 */	RLC_H,		RLC_L,		RLC_HL,		RLC_A,
/* 0x08 */	RRC_B,		RRC_C,		RRC_D,		RRC_E,
/* 0x0C */	RRC_H,		RRC_L,		RRC_HL,		RRC_A,
/* 0x10 */	RL_B,		RL_C,		RL_D,		RL_E,
/* 0x14 */	RL_H,		RL_L,		RL_HL,		RL_A,
/* 0x18 */	RR_B,		RR_C,		RR_D,		RR_E,
/* 0x1C */	RR_H,		RR_L,		RR_HL,		RR_A,
/* 0x20 */	SLA_B,		SLA_C,		SLA_D,		SLA_E,
/* 0x24 */	SLA_H,		SLA_L,		SLA_HL,		SLA_A,
/* 0x28 */	SRA_B,		SRA_C,		SRA_D,		SRA_E,
/* 0x2C */	SRA_H,		SRA_L,		SRA_HL,		SRA_A,
/* 0x30 */	SWAP_B,		SWAP_C,		SWAP_D,		SWAP_E,
/* 0x34 */	SWAP_H,		SWAP_L,		SWAP_HL,	SWAP_A,
/* 0x38 */	SRL_B,		SRL_C,		SRL_D,		SRL_E,
/* 0x3C */	SRL_H,		SRL_L,		SRL_HL,		SRL_A,
/* 0x40 */	BIT_0_B,	BIT_0_C,	BIT_0_D,	BIT_0_E,
/* 0x44 */	BIT_0_H,	BIT_0_L,	BIT_0_HL,	BIT_0_A,
/* 0x48 */	BIT_1_B,	BIT_1_C,	BIT_1_D,	BIT_1_E,
/* 0x4C */	BIT_1_H,	BIT_1_L,	BIT_1_HL,	BIT_1_A,
/* 0x50 */	BIT_2_B,	BIT_2_C,	BIT_2_D,	BIT_2_E,
/* 0x54 */	BIT_2_H,	BIT_2_L,	BIT_2_HL,	BIT_2_A,
/* 0x58 */	BIT_3_B,	BIT_3_C,	BIT_3_D,	BIT_3_E,
/* 0x5C */	BIT_3_H,	BIT_3_L,	BIT_3_HL,	BIT_3_A,
/* 0x60 */	BIT_4_B,	BIT_4_C,	BIT_4_D,	BIT_4_E,
/* 0x64 */	BIT_4_H,	BIT_4_L,	BIT_4_HL,	BIT_4_A,
/* 0x68 */	BIT_5_B,	BIT_5_C,	BIT_5_D,	BIT_5_E,
/* 0x6C */	BIT_5_H,	BIT_5_L,	BIT_5_HL,	BIT_5_A,
/* 0x70 */	BIT_6_B,	BIT_6_C,	BIT_6_D,	BIT_6_E,
/* 0x74 */	BIT_6_H,	BIT_6_L,	BIT_6_HL,	BIT_6_A,
/* 0x78 */	BIT_7_B,	BIT_7_C,	BIT_7_D,	BIT_7_E,
/* 0x7C */	BIT_7_H,	BIT_7_L,	BIT_7_HL,	BIT_7_A,
/* 0x80 */	RES_0_B,	RES_0_C,	RES_0_D,	RES_0_E,
/* 0x84 */	RES_0_H,	RES_0_L,	RES_0_HL,	RES_0_A,
/* 0x88 */	RES_1_B,	RES_1_C,	RES_1_D,	RES_1_E,
/* 0x8C */	RES_1_H,	RES_1_L,	RES_1_HL,	RES_1_A,
/* 0x90 */	RES_2_B,	RES_2_C,	RES_2_D,	RES_2_E,
/* 0x94 */	RES_2_H,	RES_2_L,	RES_2_HL,	RES_2_A,
/* 0x98 */	RES_3_B,	RES_3_C,	RES_3_D,	RES_3_E,
/* 0x9C */	RES_3_H,	RES_3_L,	RES_3_HL,	RES_3_A,
/* 0xA0 */	RES_4_B,	RES_4_C,	RES_4_D,	RES_4_E,
/* 0xA4 */	RES_4_H,	RES_4_L,	RES_4_HL,	RES_4_A,
/* 0xA8 */	RES_5_B,	RES_5_C,	RES_5_D,	RES_5_E,
/* 0xAC */	RES_5_H,	RES_5_L,	RES_5_HL,	RES_5_A,
/* 0xB0 */	RES_6_B,	RES_6_C,	RES_6_D,	RES_6_E,
/* 0xB4 */	RES_6_H,	RES_6_L,	RES_6_HL,	RES_6_A,
/* 0xB8 */	RES_7_B,	RES_7_C,	RES_7_D,	RES_7_E,
/* 0xBC */	RES_7_H,	RES_7_L,	RES_7_HL,	RES_7_A,
/* 0xC0 */	SET_0_B,	SET_0_C,	SET_0_D,	SET_0_E,
/* 0xC4 */	SET_0_H,	SET_0_L,	SET_0_HL,	SET_0_A,
/* 0xC8 */	SET_1_B,	SET_1_C,	SET_1_D,	SET_1_E,
/* 0xCC */	SET_1_H,	SET_1_L,	SET_1_HL,	SET_1_A,
/* 0xD0 */	SET_2_B,	SET_2_C,	SET_2_D,	SET_2_E,
/* 0xD4 */	SET_2_H,	SET_2_L,	SET_2_HL,	SET_2_A,
/* 0xD8 */	SET_3_B,	SET_3_C,	SET_3_D,	SET_3_E,
/* 0xDC */	SET_3_H,	SET_3_L,	SET_3_HL,	SET_3_A,
/* 0xE0 */	SET_4_B,	SET_4_C,	SET_4_D,	SET_4_E,
/* 0xE4 */	SET_4_H,	SET_4_L,	SET_4_HL,	SET_4_A,
/* 0xE8 */	SET_5_B,	SET_5_C,	SET_5_D,	SET_5_E,
/* 0xEC */	SET_5_H,	SET_5_L,	SET_5_HL,	SET_5_A,
/* 0xF0 */	SET_6_B,	SET_6_C,	SET_6_D,	SET_6_E,
/* 0xF4 */	SET_6_H,	SET_6_L,	SET_6_HL,	SET_6_A,
/* 0xF8 */	SET_7_B,	SET_7_C,	SET_7_D,	SET_7_E,
/* 0xFC */	SET_7_H,	SET_7_L,	SET_7_HL,	SET_7_A
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

void INVALID(void)
{
	printf("Invalid opcode!\n");
}
void NOP(void)
{
	return;
}
void LD_BC_d16(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_AT_BC_A(void)
{
	printf("Unimplemented opcode!\n");
}
void INC_BC(void)
{
	printf("Unimplemented opcode!\n");
}
void INC_B(void)
{
	printf("Unimplemented opcode!\n");
}
void DEC_B(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_B_d8(void)
{
	printf("Unimplemented opcode!\n");
}
void RLCA(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_a16_SP(void)
{
	printf("Unimplemented opcode!\n");
}
void ADD_HL_BC(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_A_AT_BC(void)
{
	printf("Unimplemented opcode!\n");
}
void DEC_BC(void)
{
	printf("Unimplemented opcode!\n");
}
void INC_C(void)
{
	printf("Unimplemented opcode!\n");
}
void DEC_C(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_C_d8(void)
{
	printf("Unimplemented opcode!\n");
}
void RRCA(void)
{
	printf("Unimplemented opcode!\n");
}
void STOP_0(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_DE_d16(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_AT_DE_A(void)
{
	printf("Unimplemented opcode!\n");
}
void INC_DE(void)
{
	printf("Unimplemented opcode!\n");
}
void INC_D(void)
{
	printf("Unimplemented opcode!\n");
}
void DEC_D(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_D_d8(void)
{
	printf("Unimplemented opcode!\n");
}
void RLA(void)
{
	printf("Unimplemented opcode!\n");
}
void JR_r8(void)
{
	printf("Unimplemented opcode!\n");
}
void ADD_HL_DE(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_A_AT_DE(void)
{
	printf("Unimplemented opcode!\n");
}
void DEC_DE(void)
{
	printf("Unimplemented opcode!\n");
}
void INC_E(void)
{
	printf("Unimplemented opcode!\n");
}
void DEC_E(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_E_d8(void)
{
	printf("Unimplemented opcode!\n");
}
void RRA(void)
{
	printf("Unimplemented opcode!\n");
}
void JR_NZ_r8(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_AT_HL_d16(void)
{
	printf("Unimplemented opcode!\n");
}
void LDI_AT_HL_A(void)
{
	printf("Unimplemented opcode!\n");
}
void INC_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void INC_H(void)
{
	printf("Unimplemented opcode!\n");
}
void DEC_H(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_H_d8(void)
{
	printf("Unimplemented opcode!\n");
}
void DAA(void)
{
	printf("Unimplemented opcode!\n");
}
void JR_Z_r8(void)
{
	printf("Unimplemented opcode!\n");
}
void ADD_HL_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void LDI_A_AT_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void DEC_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void INC_L(void)
{
	printf("Unimplemented opcode!\n");
}
void DEC_L(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_L_d8(void)
{
	printf("Unimplemented opcode!\n");
}
void CPL(void)
{
	printf("Unimplemented opcode!\n");
}
void JR_NC_r8(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_SP_d16(void)
{
	printf("Unimplemented opcode!\n");
}
void LDD_AT_HL_A(void)
{
	printf("Unimplemented opcode!\n");
}
void INC_SP(void)
{
	printf("Unimplemented opcode!\n");
}
void INC_AT_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void DEC_AT_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_AT_HL_d8(void)
{
	printf("Unimplemented opcode!\n");
}
void SCF(void)
{
	printf("Unimplemented opcode!\n");
}
void JR_C_r8(void)
{
	printf("Unimplemented opcode!\n");
}
void ADD_HL_SP(void)
{
	printf("Unimplemented opcode!\n");
}
void LDD_A_AT_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void DEC_SP(void)
{
	printf("Unimplemented opcode!\n");
}
void INC_A(void)
{
	printf("Unimplemented opcode!\n");
}
void DEC_A(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_A_d8(void)
{
	printf("Unimplemented opcode!\n");
}
void CCF(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_B_B(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_B_C(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_B_D(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_B_E(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_B_H(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_B_L(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_B_AT_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_B_A(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_C_B(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_C_C(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_C_D(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_C_E(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_C_H(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_C_L(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_C_AT_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_C_A(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_D_B(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_D_C(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_D_D(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_D_E(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_D_H(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_D_L(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_D_AT_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_D_A(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_E_B(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_E_C(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_E_D(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_E_E(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_E_H(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_E_L(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_E_AT_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_E_A(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_H_B(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_H_C(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_H_D(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_H_E(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_H_H(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_H_L(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_H_AT_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_H_A(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_L_B(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_L_C(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_L_D(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_L_E(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_L_H(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_L_L(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_L_AT_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_L_A(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_AT_HL_B(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_AT_HL_C(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_AT_HL_D(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_AT_HL_E(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_AT_HL_H(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_AT_HL_L(void)
{
	printf("Unimplemented opcode!\n");
}
void HALT(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_AT_HL_A(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_A_B(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_A_C(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_A_D(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_A_E(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_A_H(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_A_L(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_A_AT_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_A_A(void)
{
	printf("Unimplemented opcode!\n");
}
void ADD_A_B(void)
{
	printf("Unimplemented opcode!\n");
}
void ADD_A_C(void)
{
	printf("Unimplemented opcode!\n");
}
void ADD_A_D(void)
{
	printf("Unimplemented opcode!\n");
}
void ADD_A_E(void)
{
	printf("Unimplemented opcode!\n");
}
void ADD_A_H(void)
{
	printf("Unimplemented opcode!\n");
}
void ADD_A_L(void)
{
	printf("Unimplemented opcode!\n");
}
void ADD_A_AT_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void ADD_A_A(void)
{
	printf("Unimplemented opcode!\n");
}
void ADC_A_B(void)
{
	printf("Unimplemented opcode!\n");
}
void ADC_A_C(void)
{
	printf("Unimplemented opcode!\n");
}
void ADC_A_D(void)
{
	printf("Unimplemented opcode!\n");
}
void ADC_A_E(void)
{
	printf("Unimplemented opcode!\n");
}
void ADC_A_H(void)
{
	printf("Unimplemented opcode!\n");
}
void ADC_A_L(void)
{
	printf("Unimplemented opcode!\n");
}
void ADC_A_AT_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void ADC_A_A(void)
{
	printf("Unimplemented opcode!\n");
}
void SUB_B(void)
{
	printf("Unimplemented opcode!\n");
}
void SUB_C(void)
{
	printf("Unimplemented opcode!\n");
}
void SUB_D(void)
{
	printf("Unimplemented opcode!\n");
}
void SUB_E(void)
{
	printf("Unimplemented opcode!\n");
}
void SUB_H(void)
{
	printf("Unimplemented opcode!\n");
}
void SUB_L(void)
{
	printf("Unimplemented opcode!\n");
}
void SUB_AT_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void SUB_A(void)
{
	printf("Unimplemented opcode!\n");
}
void SBC_A_B(void)
{
	printf("Unimplemented opcode!\n");
}
void SBC_A_C(void)
{
	printf("Unimplemented opcode!\n");
}
void SBC_A_D(void)
{
	printf("Unimplemented opcode!\n");
}
void SBC_A_E(void)
{
	printf("Unimplemented opcode!\n");
}
void SBC_A_H(void)
{
	printf("Unimplemented opcode!\n");
}
void SBC_A_L(void)
{
	printf("Unimplemented opcode!\n");
}
void SBC_A_AT_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void SBC_A_A(void)
{
	printf("Unimplemented opcode!\n");
}
void AND_B(void)
{
	printf("Unimplemented opcode!\n");
}
void AND_C(void)
{
	printf("Unimplemented opcode!\n");
}
void AND_D(void)
{
	printf("Unimplemented opcode!\n");
}
void AND_E(void)
{
	printf("Unimplemented opcode!\n");
}
void AND_H(void)
{
	printf("Unimplemented opcode!\n");
}
void AND_L(void)
{
	printf("Unimplemented opcode!\n");
}
void AND_AT_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void AND_A(void)
{
	printf("Unimplemented opcode!\n");
}
void XOR_B(void)
{
	printf("Unimplemented opcode!\n");
}
void XOR_C(void)
{
	printf("Unimplemented opcode!\n");
}
void XOR_D(void)
{
	printf("Unimplemented opcode!\n");
}
void XOR_E(void)
{
	printf("Unimplemented opcode!\n");
}
void XOR_H(void)
{
	printf("Unimplemented opcode!\n");
}
void XOR_L(void)
{
	printf("Unimplemented opcode!\n");
}
void XOR_AT_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void XOR_A(void)
{
	printf("Unimplemented opcode!\n");
}
void OR_B(void)
{
	printf("Unimplemented opcode!\n");
}
void OR_C(void)
{
	printf("Unimplemented opcode!\n");
}
void OR_D(void)
{
	printf("Unimplemented opcode!\n");
}
void OR_E(void)
{
	printf("Unimplemented opcode!\n");
}
void OR_H(void)
{
	printf("Unimplemented opcode!\n");
}
void OR_L(void)
{
	printf("Unimplemented opcode!\n");
}
void OR_AT_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void OR_A(void)
{
	printf("Unimplemented opcode!\n");
}
void CP_B(void)
{
	printf("Unimplemented opcode!\n");
}
void CP_C(void)
{
	printf("Unimplemented opcode!\n");
}
void CP_D(void)
{
	printf("Unimplemented opcode!\n");
}
void CP_E(void)
{
	printf("Unimplemented opcode!\n");
}
void CP_H(void)
{
	printf("Unimplemented opcode!\n");
}
void CP_L(void)
{
	printf("Unimplemented opcode!\n");
}
void CP_AT_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void CP_A(void)
{
	printf("Unimplemented opcode!\n");
}
void RET_NZ(void)
{
	printf("Unimplemented opcode!\n");
}
void POP_BC(void)
{
	printf("Unimplemented opcode!\n");
}
void JP_NZ_a16(void)
{
	printf("Unimplemented opcode!\n");
}
void JP_a16(void)
{
	printf("Unimplemented opcode!\n");
}
void CALL_NZ_a16(void)
{
	printf("Unimplemented opcode!\n");
}
void PUSH_BC(void)
{
	printf("Unimplemented opcode!\n");
}
void ADD_A_d8(void)
{
	printf("Unimplemented opcode!\n");
}
void RST_00H(void)
{
	printf("Unimplemented opcode!\n");
}
void RET_Z(void)
{
	printf("Unimplemented opcode!\n");
}
void RET(void)
{
	printf("Unimplemented opcode!\n");
}
void JP_Z_a16(void)
{
	printf("Unimplemented opcode!\n");
}
void PREFIX_CB(void)
{
	printf("Unimplemented opcode!\n");
}
void CALL_Z_a16(void)
{
	printf("Unimplemented opcode!\n");
}
void CALL_a16(void)
{
	printf("Unimplemented opcode!\n");
}
void ADC_A_d8(void)
{
	printf("Unimplemented opcode!\n");
}
void RST_08H(void)
{
	printf("Unimplemented opcode!\n");
}
void RET_NC(void)
{
	printf("Unimplemented opcode!\n");
}
void POP_DE(void)
{
	printf("Unimplemented opcode!\n");
}
void JP_NC_a16(void)
{
	printf("Unimplemented opcode!\n");
}
void CALL_NC_a16(void)
{
	printf("Unimplemented opcode!\n");
}
void PUSH_DE(void)
{
	printf("Unimplemented opcode!\n");
}
void SUB_d8(void)
{
	printf("Unimplemented opcode!\n");
}
void RST_10H(void)
{
	printf("Unimplemented opcode!\n");
}
void RET_C(void)
{
	printf("Unimplemented opcode!\n");
}
void RETI(void)
{
	printf("Unimplemented opcode!\n");
}
void JP_C_a16(void)
{
	printf("Unimplemented opcode!\n");
}
void CALL_C_a16(void)
{
	printf("Unimplemented opcode!\n");
}
void SBC_A_d8(void)
{
	printf("Unimplemented opcode!\n");
}
void RST_18H(void)
{
	printf("Unimplemented opcode!\n");
}
void LDH_a8_A(void)
{
	printf("Unimplemented opcode!\n");
}
void POP_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_AT_C_A(void)
{
	printf("Unimplemented opcode!\n");
}
void PUSH_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void AND_d8(void)
{
	printf("Unimplemented opcode!\n");
}
void RST_20H(void)
{
	printf("Unimplemented opcode!\n");
}
void ADD_SP_r8(void)
{
	printf("Unimplemented opcode!\n");
}
void JP_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_a16_A(void)
{
	printf("Unimplemented opcode!\n");
}
void XOR_d8(void)
{
	printf("Unimplemented opcode!\n");
}
void RST_28H(void)
{
	printf("Unimplemented opcode!\n");
}
void LDH_A_a8(void)
{
	printf("Unimplemented opcode!\n");
}
void POP_AF(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_A_AT_C(void)
{
	printf("Unimplemented opcode!\n");
}
void DI(void)
{
	printf("Unimplemented opcode!\n");
}
void PUSH_AF(void)
{
	printf("Unimplemented opcode!\n");
}
void OR_d8(void)
{
	printf("Unimplemented opcode!\n");
}
void RST_30H(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_HL_SP_r8(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_SP_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void LD_A_a16(void)
{
	printf("Unimplemented opcode!\n");
}
void EI(void)
{
	printf("Unimplemented opcode!\n");
}
void CP_d8(void)
{
	printf("Unimplemented opcode!\n");
}
void RST_38H(void)
{
	printf("Unimplemented opcode!\n");
}
void RLC_B(void)
{
	printf("Unimplemented opcode!\n");
}
void RLC_C(void)
{
	printf("Unimplemented opcode!\n");
}
void RLC_D(void)
{
	printf("Unimplemented opcode!\n");
}
void RLC_E(void)
{
	printf("Unimplemented opcode!\n");
}
void RLC_H(void)
{
	printf("Unimplemented opcode!\n");
}

void RLC_L(void)
{
	printf("Unimplemented opcode!\n");
}
void RLC_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void RLC_A(void)
{
	printf("Unimplemented opcode!\n");
}
void RRC_B(void)
{
	printf("Unimplemented opcode!\n");
}
void RRC_C(void)
{
	printf("Unimplemented opcode!\n");
}
void RRC_D(void)
{
	printf("Unimplemented opcode!\n");
}
void RRC_E(void)
{
	printf("Unimplemented opcode!\n");
}
void RRC_H(void)
{
	printf("Unimplemented opcode!\n");
}
void RRC_L(void)
{
	printf("Unimplemented opcode!\n");
}
void RRC_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void RRC_A(void)
{
	printf("Unimplemented opcode!\n");
}
void RL_B(void)
{
	printf("Unimplemented opcode!\n");
}
void RL_C(void)
{
	printf("Unimplemented opcode!\n");
}
void RL_D(void)
{
	printf("Unimplemented opcode!\n");
}
void RL_E(void)
{
	printf("Unimplemented opcode!\n");
}
void RL_H(void)
{
	printf("Unimplemented opcode!\n");
}
void RL_L(void)
{
	printf("Unimplemented opcode!\n");
}
void RL_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void RL_A(void)
{
	printf("Unimplemented opcode!\n");
}
void RR_B(void)
{
	printf("Unimplemented opcode!\n");
}
void RR_C(void)
{
	printf("Unimplemented opcode!\n");
}
void RR_D(void)
{
	printf("Unimplemented opcode!\n");
}
void RR_E(void)
{
	printf("Unimplemented opcode!\n");
}
void RR_H(void)
{
	printf("Unimplemented opcode!\n");
}
void RR_L(void)
{
	printf("Unimplemented opcode!\n");
}
void RR_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void RR_A(void)
{
	printf("Unimplemented opcode!\n");
}
void SLA_B(void)
{
	printf("Unimplemented opcode!\n");
}
void SLA_C(void)
{
	printf("Unimplemented opcode!\n");
}
void SLA_D(void)
{
	printf("Unimplemented opcode!\n");
}
void SLA_E(void)
{
	printf("Unimplemented opcode!\n");
}
void SLA_H(void)
{
	printf("Unimplemented opcode!\n");
}
void SLA_L(void)
{
	printf("Unimplemented opcode!\n");
}
void SLA_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void SLA_A(void)
{
	printf("Unimplemented opcode!\n");
}
void SRA_B(void)
{
	printf("Unimplemented opcode!\n");
}
void SRA_C(void)
{
	printf("Unimplemented opcode!\n");
}
void SRA_D(void)
{
	printf("Unimplemented opcode!\n");
}
void SRA_E(void)
{
	printf("Unimplemented opcode!\n");
}
void SRA_H(void)
{
	printf("Unimplemented opcode!\n");
}
void SRA_L(void)
{
	printf("Unimplemented opcode!\n");
}
void SRA_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void SRA_A(void)
{
	printf("Unimplemented opcode!\n");
}
void SWAP_B(void)
{
	printf("Unimplemented opcode!\n");
}
void SWAP_C(void)
{
	printf("Unimplemented opcode!\n");
}
void SWAP_D(void)
{
	printf("Unimplemented opcode!\n");
}
void SWAP_E(void)
{
	printf("Unimplemented opcode!\n");
}
void SWAP_H(void)
{
	printf("Unimplemented opcode!\n");
}
void SWAP_L(void)
{
	printf("Unimplemented opcode!\n");
}
void SWAP_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void SWAP_A(void)
{
	printf("Unimplemented opcode!\n");
}
void SRL_B(void)
{
	printf("Unimplemented opcode!\n");
}
void SRL_C(void)
{
	printf("Unimplemented opcode!\n");
}
void SRL_D(void)
{
	printf("Unimplemented opcode!\n");
}
void SRL_E(void)
{
	printf("Unimplemented opcode!\n");
}
void SRL_H(void)
{
	printf("Unimplemented opcode!\n");
}
void SRL_L(void)
{
	printf("Unimplemented opcode!\n");
}
void SRL_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void SRL_A(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_0_B(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_0_C(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_0_D(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_0_E(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_0_H(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_0_L(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_0_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_0_A(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_1_B(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_1_C(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_1_D(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_1_E(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_1_H(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_1_L(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_1_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_1_A(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_2_B(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_2_C(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_2_D(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_2_E(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_2_H(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_2_L(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_2_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_2_A(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_3_B(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_3_C(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_3_D(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_3_E(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_3_H(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_3_L(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_3_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_3_A(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_4_B(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_4_C(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_4_D(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_4_E(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_4_H(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_4_L(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_4_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_4_A(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_5_B(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_5_C(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_5_D(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_5_E(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_5_H(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_5_L(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_5_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_5_A(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_6_B(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_6_C(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_6_D(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_6_E(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_6_H(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_6_L(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_6_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_6_A(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_7_B(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_7_C(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_7_D(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_7_E(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_7_H(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_7_L(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_7_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void BIT_7_A(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_0_B(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_0_C(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_0_D(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_0_E(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_0_H(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_0_L(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_0_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_0_A(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_1_B(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_1_C(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_1_D(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_1_E(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_1_H(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_1_L(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_1_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_1_A(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_2_B(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_2_C(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_2_D(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_2_E(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_2_H(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_2_L(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_2_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_2_A(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_3_B(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_3_C(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_3_D(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_3_E(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_3_H(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_3_L(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_3_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_3_A(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_4_B(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_4_C(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_4_D(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_4_E(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_4_H(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_4_L(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_4_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_4_A(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_5_B(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_5_C(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_5_D(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_5_E(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_5_H(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_5_L(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_5_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_5_A(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_6_B(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_6_C(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_6_D(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_6_E(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_6_H(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_6_L(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_6_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_6_A(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_7_B(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_7_C(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_7_D(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_7_E(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_7_H(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_7_L(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_7_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void RES_7_A(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_0_B(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_0_C(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_0_D(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_0_E(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_0_H(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_0_L(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_0_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_0_A(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_1_B(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_1_C(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_1_D(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_1_E(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_1_H(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_1_L(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_1_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_1_A(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_2_B(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_2_C(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_2_D(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_2_E(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_2_H(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_2_L(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_2_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_2_A(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_3_B(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_3_C(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_3_D(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_3_E(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_3_H(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_3_L(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_3_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_3_A(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_4_B(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_4_C(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_4_D(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_4_E(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_4_H(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_4_L(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_4_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_4_A(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_5_B(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_5_C(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_5_D(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_5_E(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_5_H(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_5_L(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_5_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_5_A(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_6_B(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_6_C(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_6_D(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_6_E(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_6_H(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_6_L(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_6_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_6_A(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_7_B(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_7_C(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_7_D(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_7_E(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_7_H(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_7_L(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_7_HL(void)
{
	printf("Unimplemented opcode!\n");
}
void SET_7_A(void)
{
	printf("Unimplemented opcode!\n");
}
