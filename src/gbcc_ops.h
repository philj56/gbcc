#ifndef GBCC_OPS_H
#define GBCC_OPS_H

#include "gbcc.h"

void NOP(struct gbc *gbc);
void LD_BC_d16(struct gbc *gbc);
void LD_AT_BC_A(struct gbc *gbc);
void INC_BC(struct gbc *gbc);

void INC_B(struct gbc *gbc);
void DEC_B(struct gbc *gbc);
void LD_B_d8(struct gbc *gbc);
void RLCA(struct gbc *gbc);

void LD_a16_SP(struct gbc *gbc);
void ADD_HL_BC(struct gbc *gbc);
void LD_A_AT_BC(struct gbc *gbc);
void DEC_BC(struct gbc *gbc);

void INC_C(struct gbc *gbc);
void DEC_C(struct gbc *gbc);
void LD_C_d8(struct gbc *gbc);
void RRCA(struct gbc *gbc);

void STOP_0(struct gbc *gbc);
void LD_DE_d16(struct gbc *gbc);
void LD_AT_DE_A(struct gbc *gbc);
void INC_DE(struct gbc *gbc);

void INC_D(struct gbc *gbc);
void DEC_D(struct gbc *gbc);
void LD_D_d8(struct gbc *gbc);
void RLA(struct gbc *gbc);

void ADD_HL_DE(struct gbc *gbc);
void LD_A_AT_DE(struct gbc *gbc);
void DEC_DE(struct gbc *gbc);

void INC_E(struct gbc *gbc);
void DEC_E(struct gbc *gbc);
void LD_E_d8(struct gbc *gbc);
void RRA(struct gbc *gbc);

void LD_AT_HL_d16(struct gbc *gbc);
void LDI_AT_HL_A(struct gbc *gbc);
void INC_HL(struct gbc *gbc);

void INC_H(struct gbc *gbc);
void DEC_H(struct gbc *gbc);
void LD_H_d8(struct gbc *gbc);
void DAA(struct gbc *gbc);

void ADD_HL_HL(struct gbc *gbc);
void LDI_A_AT_HL(struct gbc *gbc);
void DEC_HL(struct gbc *gbc);

void INC_L(struct gbc *gbc);
void DEC_L(struct gbc *gbc);
void LD_L_d8(struct gbc *gbc);
void CPL(struct gbc *gbc);

void LD_SP_d16(struct gbc *gbc);
void LDD_AT_HL_A(struct gbc *gbc);
void INC_SP(struct gbc *gbc);

void INC_AT_HL(struct gbc *gbc);
void DEC_AT_HL(struct gbc *gbc);
void LD_AT_HL_d8(struct gbc *gbc);
void SCF(struct gbc *gbc);

void ADD_HL_SP(struct gbc *gbc);
void LDD_A_AT_HL(struct gbc *gbc);
void DEC_SP(struct gbc *gbc);

void INC_A(struct gbc *gbc);
void DEC_A(struct gbc *gbc);
void LD_A_d8(struct gbc *gbc);
void CCF(struct gbc *gbc);

void LD_REG_REG(struct gbc *gbc);

void HALT(struct gbc *gbc);

void ALU_OP(struct gbc *gbc);

void RET_NZ(struct gbc *gbc);
void POP_BC(struct gbc *gbc);

void JP(struct gbc *gbc);
void JP_COND(struct gbc *gbc);
void JR(struct gbc *gbc);
void JR_COND(struct gbc *gbc);

void CALL_NZ_a16(struct gbc *gbc);
void PUSH_BC(struct gbc *gbc);
void RST_00H(struct gbc *gbc);

void RET_Z(struct gbc *gbc);
void RET(struct gbc *gbc);
void PREFIX_CB(struct gbc *gbc);

void CALL_Z_a16(struct gbc *gbc);
void CALL_a16(struct gbc *gbc);
void RST_08H(struct gbc *gbc);

void RET_NC(struct gbc *gbc);
void POP_DE(struct gbc *gbc);
void INVALID(struct gbc *gbc);

void CALL_NC_a16(struct gbc *gbc);
void PUSH_DE(struct gbc *gbc);
void RST_10H(struct gbc *gbc);

void RET_C(struct gbc *gbc);
void RETI(struct gbc *gbc);

void CALL_C_a16(struct gbc *gbc);
void RST_18H(struct gbc *gbc);

void LDH_a8_A(struct gbc *gbc);
void POP_HL(struct gbc *gbc);
void LD_AT_C_A(struct gbc *gbc);

void PUSH_HL(struct gbc *gbc);
void RST_20H(struct gbc *gbc);

void ADD_SP_r8(struct gbc *gbc);
void JP_HL(struct gbc *gbc);
void LD_a16_A(struct gbc *gbc);

void RST_28H(struct gbc *gbc);

void LDH_A_a8(struct gbc *gbc);
void POP_AF(struct gbc *gbc);
void LD_A_AT_C(struct gbc *gbc);
void DI(struct gbc *gbc);

void PUSH_AF(struct gbc *gbc);
void RST_30H(struct gbc *gbc);

void LD_HL_SP_r8(struct gbc *gbc);
void LD_SP_HL(struct gbc *gbc);
void LD_A_a16(struct gbc *gbc);
void EI(struct gbc *gbc);

void RST_38H(struct gbc *gbc);


void CB_SHIFT_OP(struct gbc *gbc);
void CB_BIT_OP(struct gbc *gbc);

#endif /* GBCC_OPS_H */
