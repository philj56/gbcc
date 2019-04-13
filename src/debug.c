#include "gbcc.h"
#include "debug.h"
#include "memory.h"
#include "ops.h"
#include <stdarg.h>
#include <stdio.h>

/* Instruction sizes, in bytes. 0 means invalid instruction */
static const uint8_t gbcc_op_sizes[0x100] = {
           /* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/* 0x00 */    1, 3, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 2, 1,
/* 0x10 */    2, 3, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1,
/* 0x20 */    2, 3, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1,
/* 0x30 */    2, 3, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1,
/* 0x40 */    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 0x50 */    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 0x60 */    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 0x70 */    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 0x80 */    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 0x90 */    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 0xA0 */    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 0xB0 */    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 0xC0 */    1, 1, 3, 3, 3, 1, 2, 1, 1, 1, 3, 1, 3, 3, 2, 1,
/* 0xD0 */    1, 1, 3, 0, 3, 1, 2, 1, 1, 1, 3, 0, 3, 0, 2, 1,
/* 0xE0 */    2, 1, 2, 0, 0, 1, 2, 1, 2, 1, 3, 0, 0, 0, 2, 1,
/* 0xF0 */    2, 1, 2, 1, 0, 1, 2, 1, 2, 1, 3, 1, 0, 0, 2, 1
};

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

static const char* const cb_op_dissassemblies[0x100] = {
/* 0x00 */	"RLC B", 	"RLC C", 	"RLC D", 	"RLC E",
/* 0x04 */	"RLC H", 	"RLC L", 	"RLC (HL)", 	"RLC A",
/* 0x08 */	"RRC B", 	"RRC C", 	"RRC D", 	"RRC E",
/* 0x0C */	"RRC H", 	"RRC L", 	"RRC (HL)", 	"RRC A",
/* 0x10 */	"RL B", 	"RL C", 	"RL D", 	"RL E",
/* 0x14 */	"RL H", 	"RL L", 	"RL (HL)", 	"RL A",
/* 0x18 */	"RR B", 	"RR C", 	"RR D", 	"RR E",
/* 0x1C */	"RR H", 	"RR L", 	"RR (HL)", 	"RR A",
/* 0x20 */	"SLA B", 	"SLA C", 	"SLA D", 	"SLA E",
/* 0x24 */	"SLA H", 	"SLA L", 	"SLA (HL)", 	"SLA A",
/* 0x28 */	"SRA B", 	"SRA C", 	"SRA D", 	"SRA E",
/* 0x2C */	"SRA H", 	"SRA L", 	"SRA (HL)", 	"SRA A",
/* 0x30 */	"SWAP B", 	"SWAP C", 	"SWAP D", 	"SWAP E",
/* 0x34 */	"SWAP H", 	"SWAP L", 	"SWAP (HL)", 	"SWAP A",
/* 0x38 */	"SRL B", 	"SRL C", 	"SRL D", 	"SRL E",
/* 0x3C */	"SRL H", 	"SRL L", 	"SRL (HL)", 	"SRL A",
/* 0x40 */	"BIT 0,B", 	"BIT 0,C", 	"BIT 0,D", 	"BIT 0,E",
/* 0x44 */	"BIT 0,H", 	"BIT 0,L", 	"BIT 0,(HL)", 	"BIT 0,A",
/* 0x48 */	"BIT 1,B", 	"BIT 1,C", 	"BIT 1,D", 	"BIT 1,E",
/* 0x4C */	"BIT 1,H", 	"BIT 1,L", 	"BIT 1,(HL)", 	"BIT 1,A",
/* 0x50 */	"BIT 2,B", 	"BIT 2,C", 	"BIT 2,D", 	"BIT 2,E",
/* 0x54 */	"BIT 2,H", 	"BIT 2,L", 	"BIT 2,(HL)", 	"BIT 2,A",
/* 0x58 */	"BIT 3,B", 	"BIT 3,C", 	"BIT 3,D", 	"BIT 3,E",
/* 0x5C */	"BIT 3,H", 	"BIT 3,L", 	"BIT 3,(HL)", 	"BIT 3,A",
/* 0x60 */	"BIT 4,B", 	"BIT 4,C", 	"BIT 4,D", 	"BIT 4,E",
/* 0x64 */	"BIT 4,H", 	"BIT 4,L", 	"BIT 4,(HL)", 	"BIT 4,A",
/* 0x68 */	"BIT 5,B", 	"BIT 5,C", 	"BIT 5,D", 	"BIT 5,E",
/* 0x6C */	"BIT 5,H", 	"BIT 5,L", 	"BIT 5,(HL)", 	"BIT 5,A",
/* 0x70 */	"BIT 6,B", 	"BIT 6,C", 	"BIT 6,D", 	"BIT 6,E",
/* 0x74 */	"BIT 6,H", 	"BIT 6,L", 	"BIT 6,(HL)", 	"BIT 6,A",
/* 0x78 */	"BIT 7,B", 	"BIT 7,C", 	"BIT 7,D", 	"BIT 7,E",
/* 0x7C */	"BIT 7,H", 	"BIT 7,L", 	"BIT 7,(HL)", 	"BIT 7,A",
/* 0x80 */	"RES 0,B", 	"RES 0,C", 	"RES 0,D", 	"RES 0,E",
/* 0x84 */	"RES 0,H", 	"RES 0,L", 	"RES 0,(HL)", 	"RES 0,A",
/* 0x88 */	"RES 1,B", 	"RES 1,C", 	"RES 1,D", 	"RES 1,E",
/* 0x8C */	"RES 1,H", 	"RES 1,L", 	"RES 1,(HL)", 	"RES 1,A",
/* 0x90 */	"RES 2,B", 	"RES 2,C", 	"RES 2,D", 	"RES 2,E",
/* 0x94 */	"RES 2,H", 	"RES 2,L", 	"RES 2,(HL)", 	"RES 2,A",
/* 0x98 */	"RES 3,B", 	"RES 3,C", 	"RES 3,D", 	"RES 3,E",
/* 0x9C */	"RES 3,H", 	"RES 3,L", 	"RES 3,(HL)", 	"RES 3,A",
/* 0xA0 */	"RES 4,B", 	"RES 4,C", 	"RES 4,D", 	"RES 4,E",
/* 0xA4 */	"RES 4,H", 	"RES 4,L", 	"RES 4,(HL)", 	"RES 4,A",
/* 0xA8 */	"RES 5,B", 	"RES 5,C", 	"RES 5,D", 	"RES 5,E",
/* 0xAC */	"RES 5,H", 	"RES 5,L", 	"RES 5,(HL)", 	"RES 5,A",
/* 0xB0 */	"RES 6,B", 	"RES 6,C", 	"RES 6,D", 	"RES 6,E",
/* 0xB4 */	"RES 6,H", 	"RES 6,L", 	"RES 6,(HL)", 	"RES 6,A",
/* 0xB8 */	"RES 7,B", 	"RES 7,C", 	"RES 7,D", 	"RES 7,E",
/* 0xBC */	"RES 7,H", 	"RES 7,L", 	"RES 7,(HL)", 	"RES 7,A",
/* 0xC0 */	"SET 0,B", 	"SET 0,C", 	"SET 0,D", 	"SET 0,E",
/* 0xC4 */	"SET 0,H", 	"SET 0,L", 	"SET 0,(HL)", 	"SET 0,A",
/* 0xC8 */	"SET 1,B", 	"SET 1,C", 	"SET 1,D", 	"SET 1,E",
/* 0xCC */	"SET 1,H", 	"SET 1,L", 	"SET 1,(HL)", 	"SET 1,A",
/* 0xD0 */	"SET 2,B", 	"SET 2,C", 	"SET 2,D", 	"SET 2,E",
/* 0xD4 */	"SET 2,H", 	"SET 2,L", 	"SET 2,(HL)", 	"SET 2,A",
/* 0xD8 */	"SET 3,B", 	"SET 3,C", 	"SET 3,D", 	"SET 3,E",
/* 0xDC */	"SET 3,H", 	"SET 3,L", 	"SET 3,(HL)", 	"SET 3,A",
/* 0xE0 */	"SET 4,B", 	"SET 4,C", 	"SET 4,D", 	"SET 4,E",
/* 0xE4 */	"SET 4,H", 	"SET 4,L", 	"SET 4,(HL)", 	"SET 4,A",
/* 0xE8 */	"SET 5,B", 	"SET 5,C", 	"SET 5,D", 	"SET 5,E",
/* 0xEC */	"SET 5,H", 	"SET 5,L", 	"SET 5,(HL)", 	"SET 5,A",
/* 0xF0 */	"SET 6,B", 	"SET 6,C", 	"SET 6,D", 	"SET 6,E",
/* 0xF4 */	"SET 6,H", 	"SET 6,L", 	"SET 6,(HL)", 	"SET 6,A",
/* 0xF8 */	"SET 7,B", 	"SET 7,C", 	"SET 7,D", 	"SET 7,E",
/* 0xFC */	"SET 7,H", 	"SET 7,L", 	"SET 7,(HL)", 	"SET 7,A",
};

void gbcc_print_registers(struct gbc *gbc)
{
	struct cpu *cpu = &gbc->cpu;
	gbcc_log_debug("Registers:\n");
	gbcc_log_debug("\ta: %u\t\taf: %04X\n", cpu->reg.a, cpu->reg.af);
	gbcc_log_debug("\tb: %u\t\tbc: %04X\n", cpu->reg.b, cpu->reg.bc);
	gbcc_log_debug("\tc: %u\t\tde: %04X\n", cpu->reg.c, cpu->reg.de);
	gbcc_log_debug("\td: %u\t\thl: %04X\n", cpu->reg.d, cpu->reg.hl);
	gbcc_log_debug("\te: %u\t\tz: %u\n", cpu->reg.e, !!(cpu->reg.f & ZF));
	gbcc_log_debug("\th: %u\t\tn: %u\n", cpu->reg.h, !!(cpu->reg.f & NF));
	gbcc_log_debug("\tl: %u\t\th: %u\n", cpu->reg.l, !!(cpu->reg.f & HF));
	gbcc_log_debug("\tsp: %04X\tc: %u\n", cpu->reg.sp, !!(cpu->reg.f & CF));
	gbcc_log_debug("\tpc: %04X\n", cpu->reg.pc);
}

void gbcc_print_op(struct gbc *gbc)
{
	struct cpu *cpu = &gbc->cpu;
	uint8_t op = cpu->opcode;
	gbcc_log_debug("%02X", op);
	for (uint8_t i = 0; i < gbcc_op_sizes[op] - 1; i++) {
		gbcc_log_append_debug("%02X", gbcc_memory_read(gbc, cpu->reg.pc + i, false));
	}
	if (op == 0xCB) {
		uint8_t cb_op = gbcc_memory_read(gbc, cpu->reg.pc, false);
		gbcc_log_append_debug("%02X", cb_op);
		gbcc_log_append_debug("\t%s\n", cb_op_dissassemblies[cb_op]);
	} else {
		gbcc_log_append_debug("\t%s\n", op_dissassemblies[op]);
	}
}

/* TODO: Clean this up */
#define RED   "\x1B[31m"
//#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
//#define MAG   "\x1B[35m"
//#define CYN   "\x1B[36m"
//#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

void gbcc_log_error(const char *const fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	fprintf(stderr, "[" RED "ERROR" RESET "]: ");
	vfprintf(stderr, fmt, args);
	va_end(args);
}

void gbcc_log_warning(const char *const fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	fprintf(stderr, "[" YEL "WARNING" RESET "]: ");
	vfprintf(stderr, fmt, args);
	va_end(args);
}

void gbcc_log_debug(const char *const fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	printf("[" BLU "DEBUG" RESET "]: ");
	vprintf(fmt, args);
	va_end(args);
}

void gbcc_log_info(const char *const fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	printf("[INFO]: ");
	vprintf(fmt, args);
	va_end(args);
}

void gbcc_log_append_error(const char *const fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
}

void gbcc_log_append_warning(const char *const fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
}

void gbcc_log_append_debug(const char *const fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

void gbcc_log_append_info(const char *const fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

void gbcc_vram_dump(struct gbc *gbc, const char *filename)
{
	FILE *fp = fopen(filename, "wbe");
	if (!fp) {
		return;
	}

	fwrite(gbc->memory.vram_bank[0], 1, VRAM_SIZE, fp);
	if (gbc->mode == GBC) {
		fwrite(gbc->memory.vram_bank[1], 1, VRAM_SIZE, fp);
	}
	fclose(fp);
}

void gbcc_sram_dump(struct gbc *gbc, const char* filename)
{
	FILE *fp = fopen(filename, "wbe");
	fwrite(gbc->cart.ram, 1, gbc->cart.ram_size, fp);
	fclose(fp);
}

/*void gbcc_print_state(struct gbc *gbc)
{
	gbcc_print_registers(gbc);
	printf("mode: %d\n", gbc->mode);
	gbcc_print_op(gbc);
	printf("IME: %d\n", gbc->ime);
	printf("halt:\n");
	printf("\tset:%d\n", gbc->halt.set);
	printf("\tno_interrupt:%d\n", gbc->halt.no_interrupt);
	printf("\tskip:%d\n", gbc->halt.skip);
	printf("DMA:\n");
	printf("\tsource:%04X\n", gbc->dma.source);
	printf("\tnew_source:%04X\n", gbc->dma.new_source);
	printf("\trequested:%d\n", gbc->dma.requested);
	printf("HDMA:\n");
	printf("\tsource:%04X\n", gbc->hdma.source);
	printf("\tdest:%04X\n", gbc->hdma.dest);
	printf("\tlength:%d\n", gbc->hdma.length);
	printf("\thblank:%d\n", gbc->hdma.hblank);
	printf("stop:%d\n", gbc->stop);
	printf("div_timer:%d\n", gbc->div_timer);
	printf("clock:%u\n", gbc->clock);
	printf("ppu_clock:%u\n", gbc->ppu_clock);
	printf("quit:%d\n", gbc->quit);
	printf("pause:%d\n", gbc->pause);
	printf("interlace:%d\n", gbc->interlace);
	printf("save_state:%d\n", gbc->save_state);
	printf("load_state:%d\n", gbc->load_state);
	printf("double_speed:%d\n", gbc->double_speed);
	printf("div_timer:%d\n", gbc->div_timer);
	// TODO: Finish
//}*/
/*
{
	
	struct palette palette;
	struct {
		struct timespec current;
		struct timespec old;
	} real_time;
	bool quit;

	
	struct {
		
		uint8_t *rom0;	
		uint8_t *romx;	
		uint8_t *vram;	
		uint8_t *sram;	
		uint8_t *wram0;	
		uint8_t *wramx;	
		uint8_t *echo;	
		uint8_t oam[OAM_SIZE];	
		uint8_t unused[UNUSED_SIZE];	
		uint8_t ioreg[IOREG_SIZE];	
		uint8_t hram[HRAM_SIZE];	
		uint8_t iereg;	
		uint8_t bgp[64]; 	
		uint8_t obp[64]; 	
		
		uint8_t wram_bank[8][WRAM0_SIZE];	
		uint8_t vram_bank[2][VRAM_SIZE]; 	
		uint32_t screen_buffer_0[GBC_SCREEN_HEIGHT * GBC_SCREEN_WIDTH];
		uint32_t screen_buffer_1[GBC_SCREEN_HEIGHT * GBC_SCREEN_WIDTH];
		struct line_buffer background_buffer;
		struct line_buffer window_buffer;
		struct line_buffer sprite_buffer;
		uint32_t *gbc_screen;
		uint32_t *sdl_screen;
		size_t frame;
	} memory;

	
	struct {
		bool a;
		bool b;
		bool start;
		bool select;
		struct {
			bool up;
			bool down;
			bool left;
			bool right;
		} dpad;
		bool turbo;
	} keys;
	
	
	struct apu apu;

	
	struct {
		const char *filename;
		uint8_t *rom;
		size_t rom_size;
		size_t rom_banks;
		uint8_t *ram;
		size_t ram_size;
		size_t ram_banks;
		bool battery;
		bool timer;
		bool rumble;
		struct gbcc_mbc {
			enum MBC type;
			enum BANK_MODE bank_mode;
			bool sram_enable;
			uint16_t romx_bank;
			uint8_t sram_bank;
			struct gbcc_rtc {
				uint8_t seconds;
				uint8_t minutes;
				uint8_t hours;
				uint8_t day_low;
				uint8_t day_high;
				uint8_t latch;
				uint8_t cur_reg;
				struct timespec base_time;
				bool mapped;
			} rtc;
		} mbc;
		char title[CART_TITLE_SIZE];
	} cart;
} */
