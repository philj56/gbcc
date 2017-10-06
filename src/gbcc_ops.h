#ifndef GBCC_OPS_H
#define GBCC_OPS_H

#include "gbcc.h"

void RLCA(struct gbc *gbc);

void LD_a16_SP(struct gbc *gbc);

void RRCA(struct gbc *gbc);

void STOP_0(struct gbc *gbc);

void RLA(struct gbc *gbc);

void RRA(struct gbc *gbc);

void DAA(struct gbc *gbc);

void CPL(struct gbc *gbc);

void SCF(struct gbc *gbc);

void CCF(struct gbc *gbc);

void RET_NZ(struct gbc *gbc);

void CALL_NZ_a16(struct gbc *gbc);
void RST_00H(struct gbc *gbc);

void RET_Z(struct gbc *gbc);
void RET(struct gbc *gbc);
void PREFIX_CB(struct gbc *gbc);

void CALL_Z_a16(struct gbc *gbc);
void CALL_a16(struct gbc *gbc);
void RST_08H(struct gbc *gbc);

void RET_NC(struct gbc *gbc);
void INVALID(struct gbc *gbc);

void CALL_NC_a16(struct gbc *gbc);
void RST_10H(struct gbc *gbc);

void RET_C(struct gbc *gbc);
void RETI(struct gbc *gbc);

void CALL_C_a16(struct gbc *gbc);
void RST_18H(struct gbc *gbc);

void LDH_a8_A(struct gbc *gbc);
void LD_AT_C_A(struct gbc *gbc);

void RST_20H(struct gbc *gbc);

void ADD_SP_r8(struct gbc *gbc);
void JP_HL(struct gbc *gbc);
void LD_a16_A(struct gbc *gbc);

void RST_28H(struct gbc *gbc);

void LDH_A_a8(struct gbc *gbc);
void LD_A_AT_C(struct gbc *gbc);
void DI(struct gbc *gbc);

void RST_30H(struct gbc *gbc);

void LD_HL_SP_r8(struct gbc *gbc);
void LD_SP_HL(struct gbc *gbc);
void LD_A_a16(struct gbc *gbc);
void EI(struct gbc *gbc);

void RST_38H(struct gbc *gbc);

/* Others */
void NOP(struct gbc *gbc);
void HALT(struct gbc *gbc);

/* Loads */
void LD_REG_REG(struct gbc *gbc);
void LD_d8(struct gbc *gbc);
void LD_d16(struct gbc *gbc);
void LD_A(struct gbc *gbc);
void PUSH_POP(struct gbc *gbc);

/* ALU */
void ALU_OP(struct gbc *gbc);
void INC_DEC_8_BIT(struct gbc *gbc);
void INC_DEC_16_BIT(struct gbc *gbc);
void ADD_HL(struct gbc *gbc);

/* Jumps */
void JP(struct gbc *gbc);
void JP_COND(struct gbc *gbc);
void JR(struct gbc *gbc);
void JR_COND(struct gbc *gbc);

/* CB-prefix */
void CB_SHIFT_OP(struct gbc *gbc);
void CB_BIT_OP(struct gbc *gbc);

#endif /* GBCC_OPS_H */
