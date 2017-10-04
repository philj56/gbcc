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

void JR_r8(struct gbc *gbc);
void ADD_HL_DE(struct gbc *gbc);
void LD_A_AT_DE(struct gbc *gbc);
void DEC_DE(struct gbc *gbc);

void INC_E(struct gbc *gbc);
void DEC_E(struct gbc *gbc);
void LD_E_d8(struct gbc *gbc);
void RRA(struct gbc *gbc);

void JR_NZ_r8(struct gbc *gbc);
void LD_AT_HL_d16(struct gbc *gbc);
void LDI_AT_HL_A(struct gbc *gbc);
void INC_HL(struct gbc *gbc);

void INC_H(struct gbc *gbc);
void DEC_H(struct gbc *gbc);
void LD_H_d8(struct gbc *gbc);
void DAA(struct gbc *gbc);

void JR_Z_r8(struct gbc *gbc);
void ADD_HL_HL(struct gbc *gbc);
void LDI_A_AT_HL(struct gbc *gbc);
void DEC_HL(struct gbc *gbc);

void INC_L(struct gbc *gbc);
void DEC_L(struct gbc *gbc);
void LD_L_d8(struct gbc *gbc);
void CPL(struct gbc *gbc);

void JR_NC_r8(struct gbc *gbc);
void LD_SP_d16(struct gbc *gbc);
void LDD_AT_HL_A(struct gbc *gbc);
void INC_SP(struct gbc *gbc);

void INC_AT_HL(struct gbc *gbc);
void DEC_AT_HL(struct gbc *gbc);
void LD_AT_HL_d8(struct gbc *gbc);
void SCF(struct gbc *gbc);

void JR_C_r8(struct gbc *gbc);
void ADD_HL_SP(struct gbc *gbc);
void LDD_A_AT_HL(struct gbc *gbc);
void DEC_SP(struct gbc *gbc);

void INC_A(struct gbc *gbc);
void DEC_A(struct gbc *gbc);
void LD_A_d8(struct gbc *gbc);
void CCF(struct gbc *gbc);

void LD_B_B(struct gbc *gbc);
void LD_B_C(struct gbc *gbc);
void LD_B_D(struct gbc *gbc);
void LD_B_E(struct gbc *gbc);

void LD_B_H(struct gbc *gbc);
void LD_B_L(struct gbc *gbc);
void LD_B_AT_HL(struct gbc *gbc);
void LD_B_A(struct gbc *gbc);

void LD_C_B(struct gbc *gbc);
void LD_C_C(struct gbc *gbc);
void LD_C_D(struct gbc *gbc);
void LD_C_E(struct gbc *gbc);

void LD_C_H(struct gbc *gbc);
void LD_C_L(struct gbc *gbc);
void LD_C_AT_HL(struct gbc *gbc);
void LD_C_A(struct gbc *gbc);

void LD_D_B(struct gbc *gbc);
void LD_D_C(struct gbc *gbc);
void LD_D_D(struct gbc *gbc);
void LD_D_E(struct gbc *gbc);

void LD_D_H(struct gbc *gbc);
void LD_D_L(struct gbc *gbc);
void LD_D_AT_HL(struct gbc *gbc);
void LD_D_A(struct gbc *gbc);

void LD_E_B(struct gbc *gbc);
void LD_E_C(struct gbc *gbc);
void LD_E_D(struct gbc *gbc);
void LD_E_E(struct gbc *gbc);

void LD_E_H(struct gbc *gbc);
void LD_E_L(struct gbc *gbc);
void LD_E_AT_HL(struct gbc *gbc);
void LD_E_A(struct gbc *gbc);

void LD_H_B(struct gbc *gbc);
void LD_H_C(struct gbc *gbc);
void LD_H_D(struct gbc *gbc);
void LD_H_E(struct gbc *gbc);

void LD_H_H(struct gbc *gbc);
void LD_H_L(struct gbc *gbc);
void LD_H_AT_HL(struct gbc *gbc);
void LD_H_A(struct gbc *gbc);

void LD_L_B(struct gbc *gbc);
void LD_L_C(struct gbc *gbc);
void LD_L_D(struct gbc *gbc);
void LD_L_E(struct gbc *gbc);

void LD_L_H(struct gbc *gbc);
void LD_L_L(struct gbc *gbc);
void LD_L_AT_HL(struct gbc *gbc);
void LD_L_A(struct gbc *gbc);

void LD_AT_HL_B(struct gbc *gbc);
void LD_AT_HL_C(struct gbc *gbc);
void LD_AT_HL_D(struct gbc *gbc);
void LD_AT_HL_E(struct gbc *gbc);

void LD_AT_HL_H(struct gbc *gbc);
void LD_AT_HL_L(struct gbc *gbc);
void HALT(struct gbc *gbc);
void LD_AT_HL_A(struct gbc *gbc);

void LD_A_B(struct gbc *gbc);
void LD_A_C(struct gbc *gbc);
void LD_A_D(struct gbc *gbc);
void LD_A_E(struct gbc *gbc);

void LD_A_H(struct gbc *gbc);
void LD_A_L(struct gbc *gbc);
void LD_A_AT_HL(struct gbc *gbc);
void LD_A_A(struct gbc *gbc);

void ADD_A_B(struct gbc *gbc);
void ADD_A_C(struct gbc *gbc);
void ADD_A_D(struct gbc *gbc);
void ADD_A_E(struct gbc *gbc);

void ADD_A_H(struct gbc *gbc);
void ADD_A_L(struct gbc *gbc);
void ADD_A_AT_HL(struct gbc *gbc);
void ADD_A_A(struct gbc *gbc);

void ADC_A_B(struct gbc *gbc);
void ADC_A_C(struct gbc *gbc);
void ADC_A_D(struct gbc *gbc);
void ADC_A_E(struct gbc *gbc);

void ADC_A_H(struct gbc *gbc);
void ADC_A_L(struct gbc *gbc);
void ADC_A_AT_HL(struct gbc *gbc);
void ADC_A_A(struct gbc *gbc);

void SUB_B(struct gbc *gbc);
void SUB_C(struct gbc *gbc);
void SUB_D(struct gbc *gbc);
void SUB_E(struct gbc *gbc);

void SUB_H(struct gbc *gbc);
void SUB_L(struct gbc *gbc);
void SUB_AT_HL(struct gbc *gbc);
void SUB_A(struct gbc *gbc);

void SBC_A_B(struct gbc *gbc);
void SBC_A_C(struct gbc *gbc);
void SBC_A_D(struct gbc *gbc);
void SBC_A_E(struct gbc *gbc);

void SBC_A_H(struct gbc *gbc);
void SBC_A_L(struct gbc *gbc);
void SBC_A_AT_HL(struct gbc *gbc);
void SBC_A_A(struct gbc *gbc);

void AND_B(struct gbc *gbc);
void AND_C(struct gbc *gbc);
void AND_D(struct gbc *gbc);
void AND_E(struct gbc *gbc);

void AND_H(struct gbc *gbc);
void AND_L(struct gbc *gbc);
void AND_AT_HL(struct gbc *gbc);
void AND_A(struct gbc *gbc);

void XOR_B(struct gbc *gbc);
void XOR_C(struct gbc *gbc);
void XOR_D(struct gbc *gbc);
void XOR_E(struct gbc *gbc);

void XOR_H(struct gbc *gbc);
void XOR_L(struct gbc *gbc);
void XOR_AT_HL(struct gbc *gbc);
void XOR_A(struct gbc *gbc);

void OR_B(struct gbc *gbc);
void OR_C(struct gbc *gbc);
void OR_D(struct gbc *gbc);
void OR_E(struct gbc *gbc);

void OR_H(struct gbc *gbc);
void OR_L(struct gbc *gbc);
void OR_AT_HL(struct gbc *gbc);
void OR_A(struct gbc *gbc);

void CP_B(struct gbc *gbc);
void CP_C(struct gbc *gbc);
void CP_D(struct gbc *gbc);
void CP_E(struct gbc *gbc);

void CP_H(struct gbc *gbc);
void CP_L(struct gbc *gbc);
void CP_AT_HL(struct gbc *gbc);
void CP_A(struct gbc *gbc);

void RET_NZ(struct gbc *gbc);
void POP_BC(struct gbc *gbc);
void JP_NZ_a16(struct gbc *gbc);
void JP_a16(struct gbc *gbc);

void CALL_NZ_a16(struct gbc *gbc);
void PUSH_BC(struct gbc *gbc);
void ADD_A_d8(struct gbc *gbc);
void RST_00H(struct gbc *gbc);

void RET_Z(struct gbc *gbc);
void RET(struct gbc *gbc);
void JP_Z_a16(struct gbc *gbc);
void PREFIX_CB(struct gbc *gbc);

void CALL_Z_a16(struct gbc *gbc);
void CALL_a16(struct gbc *gbc);
void ADC_A_d8(struct gbc *gbc);
void RST_08H(struct gbc *gbc);

void RET_NC(struct gbc *gbc);
void POP_DE(struct gbc *gbc);
void JP_NC_a16(struct gbc *gbc);
void INVALID(struct gbc *gbc);

void CALL_NC_a16(struct gbc *gbc);
void PUSH_DE(struct gbc *gbc);
void SUB_d8(struct gbc *gbc);
void RST_10H(struct gbc *gbc);

void RET_C(struct gbc *gbc);
void RETI(struct gbc *gbc);
void JP_C_a16(struct gbc *gbc);

void CALL_C_a16(struct gbc *gbc);
void SBC_A_d8(struct gbc *gbc);
void RST_18H(struct gbc *gbc);

void LDH_a8_A(struct gbc *gbc);
void POP_HL(struct gbc *gbc);
void LD_AT_C_A(struct gbc *gbc);

void PUSH_HL(struct gbc *gbc);
void AND_d8(struct gbc *gbc);
void RST_20H(struct gbc *gbc);

void ADD_SP_r8(struct gbc *gbc);
void JP_HL(struct gbc *gbc);
void LD_a16_A(struct gbc *gbc);

void XOR_d8(struct gbc *gbc);
void RST_28H(struct gbc *gbc);

void LDH_A_a8(struct gbc *gbc);
void POP_AF(struct gbc *gbc);
void LD_A_AT_C(struct gbc *gbc);
void DI(struct gbc *gbc);

void PUSH_AF(struct gbc *gbc);
void OR_d8(struct gbc *gbc);
void RST_30H(struct gbc *gbc);

void LD_HL_SP_r8(struct gbc *gbc);
void LD_SP_HL(struct gbc *gbc);
void LD_A_a16(struct gbc *gbc);
void EI(struct gbc *gbc);

void CP_d8(struct gbc *gbc);
void RST_38H(struct gbc *gbc);


void CB_SHIFT_OP(struct gbc *gbc);
void CB_BIT_OP(struct gbc *gbc);

#endif /* GBCC_OPS_H */
