#include "gbcc.h"
#include "gbcc_debug.h"
#include "gbcc_memory.h"
#include "gbcc_ops.h"
#include <stdarg.h>
#include <stdio.h>

static const char* const op_dissassemblies[0x100] = {
/* 0x00 */	"NOP",		"LD BC,d16",	"LD (BC),A",	"INC BC",
/* 0x04 */	"INC B",	"DEC B",	"LD B,d8",	"RLCA",
/* 0x08 */	"LD (a16),SP",	"ADD HL,BC",	"LD A,(BC)",	"DEC BC",
/* 0x0C */	"INC C",	"DEC C",	"LD C,d8",	"RRCA",
/* 0x10 */	"STOP 0",	"LD DE,d16",	"LD (DE),A",	"INC DE",
/* 0x14 */	"INC D",	"DEC D",	"LD D,d8",	"RLA",
/* 0x18 */	"JR r8",	"ADD HL,DE",	"LD A,(DE)",	"DEC DE",
/* 0x1C */	"INC E",	"DEC E",	"LD E,d8",	"RRA",
/* 0x20 */	"JR NZ,r8",	"LD HL,d16",	"LD (HL+),A",	"INC HL",
/* 0x24 */	"INC H",	"DEC H",	"LD H,d8",	"DAA",
/* 0x28 */	"JR Z,r8",	"ADD HL,HL",	"LD A,(HL+)",	"DEC HL",
/* 0x2C */	"INC L",	"DEC L",	"LD L,d8",	"CPL",
/* 0x30 */	"JR NC,r8",	"LD SP,d16",	"LD (HL-),A",	"INC SP",
/* 0x34 */	"INC (HL)",	"DEC (HL)",	"LD (HL),d8",	"SCF",
/* 0x38 */	"JR C,r8",	"ADD HL,SP",	"LD A,(HL-)",	"DEC SP",
/* 0x3C */	"INC A",	"DEC A",	"LD A,d8",	"CCF",
/* 0x40 */	"LD B,B",	"LD B,C",	"LD B,D",	"LD B,E",
/* 0x44 */	"LD B,H",	"LD B,L",	"LD B,(HL)",	"LD B,A",
/* 0x48 */	"LD C,B",	"LD C,C",	"LD C,D",	"LD C,E",
/* 0x4C */	"LD C,H",	"LD C,L",	"LD C,(HL)",	"LD C,A",
/* 0x50 */	"LD D,B",	"LD D,C",	"LD D,D",	"LD D,E",
/* 0x54 */	"LD D,H",	"LD D,L",	"LD D,(HL)",	"LD D,A",
/* 0x58 */	"LD E,B",	"LD E,C",	"LD E,D",	"LD E,E",
/* 0x5C */	"LD E,H",	"LD E,L",	"LD E,(HL)",	"LD E,A",
/* 0x60 */	"LD H,B",	"LD H,C",	"LD H,D",	"LD H,E",
/* 0x64 */	"LD H,H",	"LD H,L",	"LD H,(HL)",	"LD H,A",
/* 0x68 */	"LD L,B",	"LD L,C",	"LD L,D",	"LD L,E",
/* 0x6C */	"LD L,H",	"LD L,L",	"LD L,(HL)",	"LD L,A",
/* 0x70 */	"LD (HL),B",	"LD (HL),C",	"LD (HL),D",	"LD (HL),E",
/* 0x74 */	"LD (HL),H",	"LD (HL),L",	"HALT",	"LD (HL),A",
/* 0x78 */	"LD A,B",	"LD A,C",	"LD A,D",	"LD A,E",
/* 0x7C */	"LD A,H",	"LD A,L",	"LD A,(HL)",	"LD A,A",
/* 0x80 */	"ADD A,B",	"ADD A,C",	"ADD A,D",	"ADD A,E",
/* 0x84 */	"ADD A,H",	"ADD A,L",	"ADD A,(HL)",	"ADD A,A",
/* 0x88 */	"ADC A,B",	"ADC A,C",	"ADC A,D",	"ADC A,E",
/* 0x8C */	"ADC A,H",	"ADC A,L",	"ADC A,(HL)",	"ADC A,A",
/* 0x90 */	"SUB B",	"SUB C",	"SUB D",	"SUB E",
/* 0x94 */	"SUB H",	"SUB L",	"SUB (HL)",	"SUB A",
/* 0x98 */	"SBC A,B",	"SBC A,C",	"SBC A,D",	"SBC A,E",
/* 0x9C */	"SBC A,H",	"SBC A,L",	"SBC A,(HL)",	"SBC A,A",
/* 0xA0 */	"AND B",	"AND C",	"AND D",	"AND E",
/* 0xA4 */	"AND H",	"AND L",	"AND (HL)",	"AND A",
/* 0xA8 */	"XOR B",	"XOR C",	"XOR D",	"XOR E",
/* 0xAC */	"XOR H",	"XOR L",	"XOR (HL)",	"XOR A",
/* 0xB0 */	"OR B",		"OR C",		"OR D",		"OR E",
/* 0xB4 */	"OR H",		"OR L",		"OR (HL)",	"OR A",
/* 0xB8 */	"CP B",		"CP C",		"CP D",		"CP E",
/* 0xBC */	"CP H",		"CP L",		"CP (HL)",	"CP A",
/* 0xC0 */	"RET NZ",	"POP BC",	"JP NZ,a16",	"JP a16",
/* 0xC4 */	"CALL NZ,a16",	"PUSH BC",	"ADD A,d8",	"RST 00H",
/* 0xC8 */	"RET Z",	"RET",		"JP Z,a16",	"PREFIX CB",
/* 0xCC */	"CALL Z,a16",	"CALL a16",	"ADC A,d8",	"RST 08H",
/* 0xD0 */	"RET NC",	"POP DE",	"JP NC,a16",	"",
/* 0xD4 */	"CALL NC,a16",	"PUSH DE",	"SUB d8",	"RST 10H",
/* 0xD8 */	"RET C",	"RETI",		"JP C,a16",	"",
/* 0xDC */	"CALL C,a16",	"",		"SBC A,d8",	"RST 18H",
/* 0xE0 */	"LDH (a8),A",	"POP HL",	"LD (C),A",	"",
/* 0xE4 */	"",		"PUSH HL",	"AND d8",	"RST 20H",
/* 0xE8 */	"ADD SP,r8",	"JP (HL)",	"LD (a16),A",	"",
/* 0xEC */	"",		"",		"XOR d8",	"RST 28H",
/* 0xF0 */	"LDH A,(a8)",	"POP AF",	"LD A,(C)",	"DI",
/* 0xF4 */	"",		"PUSH AF",	"OR d8",	"RST 30H",
/* 0xF8 */	"LD HL,SP+r8",	"LD SP,HL",	"LD A,(a16)",	"EI",
/* 0xFC */	"",		"",		"CP d8",	"RST 38H"
};

void gbcc_print_registers(struct gbc *gbc) {
	gbcc_log(GBCC_LOG_DEBUG, "Registers:\n");
	gbcc_log(GBCC_LOG_DEBUG, "\ta: %u\t\taf: %04X\n", gbc->reg.a, gbc->reg.af);
	gbcc_log(GBCC_LOG_DEBUG, "\tb: %u\t\tbc: %04X\n", gbc->reg.b, gbc->reg.bc);
	gbcc_log(GBCC_LOG_DEBUG, "\tc: %u\t\tde: %04X\n", gbc->reg.c, gbc->reg.de);
	gbcc_log(GBCC_LOG_DEBUG, "\td: %u\t\thl: %04X\n", gbc->reg.d, gbc->reg.hl);
	gbcc_log(GBCC_LOG_DEBUG, "\te: %u\t\tz: %u\n", gbc->reg.e, !!(gbc->reg.f & ZF));
	gbcc_log(GBCC_LOG_DEBUG, "\th: %u\t\tn: %u\n", gbc->reg.h, !!(gbc->reg.f & NF));
	gbcc_log(GBCC_LOG_DEBUG, "\tl: %u\t\th: %u\n", gbc->reg.l, !!(gbc->reg.f & HF));
	gbcc_log(GBCC_LOG_DEBUG, "\tsp: %04X\tc: %u\n", gbc->reg.sp, !!(gbc->reg.f & CF));
	gbcc_log(GBCC_LOG_DEBUG, "\tpc: %04X\n", gbc->reg.pc);
}

void gbcc_print_op(struct gbc *gbc) {
	gbcc_log(GBCC_LOG_DEBUG, "%02X", gbc->opcode);
	for (uint8_t i = 0; i < gbcc_op_sizes[gbc->opcode] - 1; i++) {
		gbcc_log_append(GBCC_LOG_DEBUG, "%02X", gbcc_memory_read(gbc, gbc->reg.pc + i, true));
	}
	gbcc_log_append(GBCC_LOG_DEBUG, "\t%s\n", op_dissassemblies[gbc->opcode]);
}

/* TODO: Clean this up */
#define RED   "\x1B[31m"
//#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
//#define BLU   "\x1B[34m"
//#define MAG   "\x1B[35m"
//#define CYN   "\x1B[36m"
//#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

void gbcc_log(enum GBCC_LOG_LEVEL level, const char *const fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	switch (level) {
		case GBCC_LOG_ERROR:
			fprintf(stderr, "[" RED "ERROR" RESET "]: ");
			vfprintf(stderr, fmt, args);
			break;
		case GBCC_LOG_DEBUG:
			printf("[" YEL "DEBUG" RESET "]: ");
			vprintf(fmt, args);
			break;
		case GBCC_LOG_INFO:
			printf("[INFO]: ");
			vprintf(fmt, args);
			break;
	}
	va_end(args);
}

void gbcc_log_append(enum GBCC_LOG_LEVEL level, const char *const fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	switch (level) {
		case GBCC_LOG_ERROR:
			vfprintf(stderr, fmt, args);
			break;
		case GBCC_LOG_DEBUG:
			vprintf(fmt, args);
			break;
		case GBCC_LOG_INFO:
			vprintf(fmt, args);
			break;
	}
	va_end(args);
}

void gbcc_vram_dump(struct gbc *gbc, const char *filename)
{
	FILE *fp = fopen(filename, "wbe");
	fwrite(gbc->memory.vram_bank[0], 1, VRAM_SIZE, fp);
	if (gbc->mode == GBC) {
		fwrite(gbc->memory.vram_bank[1], 1, VRAM_SIZE, fp);
	}
	fclose(fp);
}
