#include "gbcc.h"
#include "gbcc_constants.h"
#include "gbcc_debug.h"
#include "gbcc_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

static const uint8_t nintendo_logo[CART_LOGO_SIZE] = {
	0xCEu, 0xEDu, 0x66u, 0x66u, 0xCCu, 0x0Du, 0x00u, 0x0Bu,
	0x03u, 0x73u, 0x00u, 0x83u, 0x00u, 0x0Cu, 0x00u, 0x0Du,
	0x00u, 0x08u, 0x11u, 0x1Fu, 0x88u, 0x89u, 0x00u, 0x0Eu,
	0xDCu, 0xCCu, 0x6Eu, 0xE6u, 0xDDu, 0xDDu, 0xD9u, 0x99u,
	0xBBu, 0xBBu, 0x67u, 0x63u, 0x6Eu, 0x0Eu, 0xECu, 0xCCu,
	0xDDu, 0xDCu, 0x99u, 0x9Fu, 0xBBu, 0xB9u, 0x33u, 0x3Eu
};

static void gbcc_load_rom(struct gbc *gbc, const char *filename);
static void gbcc_parse_header(struct gbc *gbc);
static void gbcc_verify_cartridge(struct gbc *gbc);
static void gbcc_print_title(struct gbc *gbc);
static void gbcc_init_mode(struct gbc *gbc);
static void gbcc_print_licensee_code(struct gbc *gbc);
static void gbcc_get_cartridge_hardware(struct gbc *gbc);
static void gbcc_init_ram(struct gbc *gbc);
static void gbcc_print_destination_code(struct gbc *gbc);
static void gbcc_init_registers(struct gbc *gbc);
static void gbcc_init_mmap(struct gbc *gbc);
static void gbcc_init_ioreg(struct gbc *gbc);
static void gbcc_init_input(struct gbc *gbc);

void gbcc_initialise(struct gbc *gbc, const char *filename)
{
	gbc->cart.rom = NULL;
	gbc->cart.ram = NULL;
	gbc->cart.mbc.type = NONE;
	gbc->cart.mbc.sram_enable = false;
	gbc->cart.mbc.romx_bank = 0x01u;
	gbc->cart.mbc.sram_bank = 0x00u;
	gbc->cart.mbc.bank_mode = 0x00u;
	gbc->cart.battery = false;
	gbc->cart.timer = false;
	gbc->cart.rumble = false;
	gbc->cart.ram_size = 0;
	gbc->mode = DMG;
	gbc->ime = true;
	gbc->halt.set = false;
	gbc->halt.no_interrupt = false;
	gbc->halt.skip = 0;
	gbc->dma.source = 0;
	gbc->dma.timer = 0;
	gbc->stop = false;
	gbc->instruction_timer = 0;
	gbc->clock = 0;
	gbcc_load_rom(gbc, filename);
	gbcc_parse_header(gbc);
	gbcc_init_mmap(gbc);
	gbcc_init_ioreg(gbc);
	gbcc_init_input(gbc);
}

void gbcc_free(struct gbc *gbc)
{
	free(gbc->cart.rom);
	if (gbc->cart.ram_size > 0) {
		free(gbc->cart.ram);
	}
	free(gbc->memory.emu_wram);
	free(gbc->memory.emu_vram);
}

void gbcc_load_rom(struct gbc *gbc, const char *filename)
{
	gbcc_log(GBCC_LOG_INFO, "Loading %s...\n", filename);
	size_t read;
	uint8_t rom_size_flag;

	FILE *rom = fopen(filename, "rbe");
	if (rom == NULL)
	{
		gbcc_log(GBCC_LOG_ERROR, "Error opening file: %s\n", filename);
		exit(1);
	}

	fseek(rom, CART_ROM_SIZE_FLAG, SEEK_SET);
	if (ferror(rom)) {
		gbcc_log(GBCC_LOG_ERROR, "Error seeking in file: %s\n", filename);
		fclose(rom);
		exit(1);
	}

	read = fread(&rom_size_flag, 1, 1, rom);
	if (read == 0) {
		gbcc_log(GBCC_LOG_ERROR, "Error reading ROM size from: %s\n", filename);
		fclose(rom);
		exit(1);
	}

	if (rom_size_flag < 0x08u) {
		gbc->cart.rom_size = 0x8000u << rom_size_flag; /* 32KB shl N */
	} else if (rom_size_flag == 0x52u) {
		gbc->cart.rom_size = 0x120000u;
	} else if (rom_size_flag == 0x53u) {
		gbc->cart.rom_size = 0x140000u;
	} else if (rom_size_flag == 0x54u) {
		gbc->cart.rom_size = 0x180000u;
	} else {
		gbcc_log(GBCC_LOG_ERROR, "Unknown ROM size flag: %u\n", rom_size_flag);
		fclose(rom);
		exit(1);
	}
	gbcc_log(GBCC_LOG_INFO, "\tCartridge size: %lu bytes\n", gbc->cart.rom_size);

	gbc->cart.rom = (uint8_t *) calloc(gbc->cart.rom_size, 1);
	if (gbc->cart.rom == NULL) {
		gbcc_log(GBCC_LOG_ERROR, "Error allocating ROM.\n");
		fclose(rom);
		exit(1);
	}


	if (fseek(rom, 0, SEEK_SET) != 0) {
		gbcc_log(GBCC_LOG_ERROR, "Error seeking in file: %s\n", filename);
		fclose(rom);
		exit(1);
	}

	read = fread(gbc->cart.rom, 1, gbc->cart.rom_size, rom);
	if (read == 0) {
		gbcc_log(GBCC_LOG_ERROR, "Error reading from file: %s\n", filename);
		fclose(rom);
		exit(1);
	}

	fclose(rom);
	gbcc_log(GBCC_LOG_INFO, "\tROM loaded.\n");
}

void gbcc_parse_header(struct gbc *gbc)
{
	gbcc_log(GBCC_LOG_INFO, "Parsing header...\n");
	gbcc_verify_cartridge(gbc);
	gbcc_print_title(gbc);
	gbcc_print_licensee_code(gbc);
	//gbcc_init_mode(gbc);
	gbcc_get_cartridge_hardware(gbc);
	gbcc_init_ram(gbc);
	gbcc_print_destination_code(gbc);
	gbcc_init_registers(gbc);
	gbcc_log(GBCC_LOG_INFO, "\tHeader parsed.\n");
}

void gbcc_verify_cartridge(struct gbc *gbc)
{
	for (size_t i = CART_LOGO_START; i < CART_LOGO_GBC_CHECK_END; i++) {
		if (gbc->cart.rom[i] != nintendo_logo[i - CART_LOGO_START]) {
			gbcc_log(GBCC_LOG_ERROR, "Cartridge logo check failed on byte %04X\n", i);
			exit(1);
		}
	}
	gbcc_log(GBCC_LOG_INFO, "\tCartridge logo check passed.\n");
	uint16_t sum = 0;
	for (size_t i = CART_HEADER_CHECKSUM_START; i < CART_HEADER_CHECKSUM_START + CART_HEADER_CHECKSUM_SIZE + 1; i++) {
		sum += gbc->cart.rom[i];
	}
	sum = (sum + 25u) & 0x00FFu;
	if (sum) {
		gbcc_log(GBCC_LOG_ERROR, "Cartridge checksum failed with value %04X.\n", sum);
		exit(1);
	}	
	gbcc_log(GBCC_LOG_INFO, "\tCartridge checksum passed.\n");
}

void gbcc_print_title(struct gbc *gbc)
{
	unsigned char title[CART_TITLE_SIZE];
	for (size_t i = CART_TITLE_START; i < CART_TITLE_END; i++) {
		title[i - CART_TITLE_START] = gbc->cart.rom[i];
	}
	gbcc_log(GBCC_LOG_INFO, "\tTitle: %s\n", title);
}

void gbcc_print_licensee_code(struct gbc *gbc)
{
	uint8_t old_code = gbc->cart.rom[CART_OLD_LICENSEE_CODE];
	char buffer[CART_NEW_LICENSEE_CODE_SIZE + 21];
	size_t len = sizeof(buffer) / sizeof(buffer[0]);
	if (old_code != 0x33u) {
		snprintf(buffer, len, "\tLicensee code: 0x%02X\n", old_code);
	} else {
		unsigned char new_code[CART_NEW_LICENSEE_CODE_SIZE + 1];
		for (size_t i = CART_NEW_LICENSEE_CODE_START; i < CART_NEW_LICENSEE_CODE_END; i++) {
			new_code[i - CART_NEW_LICENSEE_CODE_START] = gbc->cart.rom[i];
		}
		new_code[CART_NEW_LICENSEE_CODE_SIZE] = '\0';
		snprintf(buffer, len, "\tLicensee code: %s\n", new_code);
	}
	gbcc_log(GBCC_LOG_INFO, buffer);
}

void gbcc_init_mode(struct gbc *gbc)
{
	uint8_t mode_flag;

	mode_flag = gbc->cart.rom[CART_GBC_FLAG];
	if (mode_flag == 0x80u || mode_flag == 0xC0u) {
		gbc->mode = GBC;
	}
	if ((mode_flag & (1u << 6u)) && (mode_flag & (3u << 2u))) {
		/* TODO: Handle this */
	}
	switch (gbc->mode) {
		case DMG:
			gbcc_log(GBCC_LOG_INFO, "\tMode: DMG\n");
			break;
		case GBC:
			gbcc_log(GBCC_LOG_INFO, "\tMode: GBC\n");
			break;
	}
}

void gbcc_get_cartridge_hardware(struct gbc *gbc)
{
	switch (gbc->cart.rom[CART_TYPE]) {
		case 0x00u:	/* ROM ONLY */
			gbc->cart.mbc.type = NONE;
			gbcc_log(GBCC_LOG_INFO, "\tHardware: ROM only\n");
			break;
		case 0x01u:	/* MBC1 */
			gbc->cart.mbc.type = MBC1;
			gbcc_log(GBCC_LOG_INFO, "\tHardware: MBC1\n");
			break;
		case 0x02u:	/* MBC1 + RAM */
			gbc->cart.mbc.type = MBC1;
			gbcc_log(GBCC_LOG_INFO, "\tHardware: MBC1 + RAM\n");
			break;
		case 0x03u:	/* MBC1 + RAM + BATTERY */
			gbc->cart.mbc.type = MBC1;
			gbc->cart.battery = true;
			gbcc_log(GBCC_LOG_INFO, "\tHardware: MBC1 + RAM + Battery\n");
			break;
		case 0x05u:	/* MBC2 */
			gbc->cart.mbc.type = MBC2;
			gbcc_log(GBCC_LOG_INFO, "\tHardware: MBC2\n");
			break;
		case 0x06u:	/* MBC2 + BATTERY */
			gbc->cart.mbc.type = MBC2;
			gbc->cart.battery = true;
			gbcc_log(GBCC_LOG_INFO, "\tHardware: MBC2 + Battery\n");
			break;
		case 0x08u:	/* ROM + RAM */
			gbcc_log(GBCC_LOG_INFO, "\tHardware: ROM + RAM\n");
			break;
		case 0x09u:	/* ROM + RAM + BATTERY */
			gbc->cart.battery = true;
			gbcc_log(GBCC_LOG_INFO, "\tHardware: ROM + RAM + Battery\n");
			break;
		case 0x0Bu:	/* MMM01 */
			gbc->cart.mbc.type = MMM01;
			gbcc_log(GBCC_LOG_INFO, "\tHardware: ROM + RAM + Battery\n");
			break;
		case 0x0Cu:	/* MMM01 + RAM */
			gbc->cart.mbc.type = MMM01;
			gbcc_log(GBCC_LOG_INFO, "\tHardware: MMM01\n");
			break;
		case 0x0Du:	/* MMM01 + RAM + BATTERY */
			gbc->cart.mbc.type = MMM01;
			gbc->cart.battery = true;
			gbcc_log(GBCC_LOG_INFO, "\tHardware: MMM01 + Battery\n");
			break;
		case 0x0Fu:	/* MBC3 + TIMER + BATTERY */
			gbc->cart.mbc.type = MBC3;
			gbc->cart.timer = true;
			gbc->cart.battery = true;
			gbcc_log(GBCC_LOG_INFO, "\tHardware: MBC3 + Timer + Battery\n");
			break;
		case 0x10u:	/* MBC3 + TIMER + RAM + BATTERY */
			gbc->cart.timer = true;
			gbc->cart.battery = true;
			gbc->cart.mbc.type = MBC3;
			gbcc_log(GBCC_LOG_INFO, "\tHardware: MBC3 + Timer + RAM + Battery\n");
			break;
		case 0x11u:	/* MBC3 */
			gbc->cart.mbc.type = MBC3;
			gbcc_log(GBCC_LOG_INFO, "\tHardware: MBC3\n");
			break;
		case 0x12u:	/* MBC3 + RAM */
			gbc->cart.mbc.type = MBC3;
			gbcc_log(GBCC_LOG_INFO, "\tHardware: MBC3 + RAM\n");
			break;
		case 0x13u:	/* MBC3 + RAM + BATTERY */
			gbc->cart.mbc.type = MBC3;
			gbc->cart.battery = true;
			gbcc_log(GBCC_LOG_INFO, "\tHardware: MBC3 + RAM + Battery\n");
			break;
		case 0x15u:	/* MBC4 */
			gbc->cart.mbc.type = MBC4;
			gbcc_log(GBCC_LOG_INFO, "\tHardware: MBC4\n");
			break;
		case 0x16u:	/* MBC4 + RAM */
			gbc->cart.mbc.type = MBC4;
			gbcc_log(GBCC_LOG_INFO, "\tHardware: MBC4 + RAM\n");
			break;
		case 0x17u:	/* MBC4 + RAM + BATTERY */
			gbc->cart.mbc.type = MBC4;
			gbc->cart.battery = true;
			gbcc_log(GBCC_LOG_INFO, "\tHardware: MBC4 + RAM + Battery\n");
			break;
		case 0x19u:	/* MBC5 */
			gbc->cart.mbc.type = MBC5;
			gbcc_log(GBCC_LOG_INFO, "\tHardware: MBC5\n");
			break;
		case 0x1Au:	/* MBC5 + RAM */
			gbc->cart.mbc.type = MBC5;
			gbcc_log(GBCC_LOG_INFO, "\tHardware: MBC5 + RAM\n");
			break;
		case 0x1Bu:	/* MBC5 + RAM + BATTERY */
			gbc->cart.mbc.type = MBC5;
			gbc->cart.battery = true;
			gbcc_log(GBCC_LOG_INFO, "\tHardware: MBC5 + RAM + Battery\n");
			break;
		case 0x1Cu:	/* MBC5 + RUMBLE */
			gbc->cart.mbc.type = MBC5;
			gbc->cart.rumble = true;
			gbcc_log(GBCC_LOG_INFO, "\tHardware: MBC5 + Rumble\n");
			break;
		case 0x1Du:	/* MBC5 + RUMBLE + RAM */
			gbc->cart.mbc.type = MBC5;
			gbc->cart.rumble = true;
			gbcc_log(GBCC_LOG_INFO, "\tHardware: MBC5 + Rumble + RAM\n");
			break;
		case 0x1Eu:	/* MBC5 + RUMBLE + RAM + BATTERY */
			gbc->cart.mbc.type = MBC5;
			gbc->cart.rumble = true;
			gbc->cart.battery = true;
			gbcc_log(GBCC_LOG_INFO, "\tHardware: MBC5 + Rumble + RAM + Battery\n");
			break;
		default:
			gbcc_log(GBCC_LOG_ERROR, "Unrecognised hardware %02X\n", gbc->cart.rom[CART_TYPE]);
			/* TODO: Handle Pocket Camera, TAMA5, HuC3 & HuC1 */
			break;
	}
}

void gbcc_init_ram(struct gbc *gbc)
{
	uint8_t ram_size_flag;

	ram_size_flag = gbc->cart.rom[CART_RAM_SIZE_FLAG];

	switch (ram_size_flag) {
		case 0x00u:
			if (gbc->cart.mbc.type == MBC2) {
				gbc->cart.ram_size = 0x0200u;
			}
			break;
		case 0x01u:
			gbc->cart.ram_size = 0x0800u;
			break;
		case 0x02u:
			gbc->cart.ram_size = 0x2000u;
			break;
		case 0x03u:
			gbc->cart.ram_size = 0x8000u;
			break;
		default:
			gbcc_log(GBCC_LOG_ERROR, "Unknown ram size flag: %u\n", ram_size_flag);
			exit(1);
	}

	/*if (gbc->cart.mbc.type == NONE) {
		gbc->cart.ram_size = 0x0200u;
	}*/

	if (gbc->cart.ram_size > 0) {
		gbcc_log(GBCC_LOG_INFO, "\tCartridge RAM: %lu bytes\n", gbc->cart.ram_size);
		gbc->cart.ram = (uint8_t *) calloc(gbc->cart.ram_size, 1);
		if (gbc->cart.ram == NULL) {
			gbcc_log(GBCC_LOG_ERROR, "Error allocating RAM.\n");
			exit(1);
		}
	}
}

void gbcc_print_destination_code(struct gbc *gbc)
{
	uint8_t dest_code = gbc->cart.rom[CART_DESTINATION_CODE];
	switch (dest_code) {
		case 0x00u:
			gbcc_log(GBCC_LOG_INFO, "\tRegion: Japan\n");
			break;
		case 0x01u:
			gbcc_log(GBCC_LOG_INFO, "\tRegion: Non-Japanese\n");
			break;
		default:
			gbcc_log(GBCC_LOG_ERROR, "Unrecognised region code: 0x%02X\n", dest_code);
			exit(1);
	}
}

void gbcc_init_registers(struct gbc *gbc) {
	switch (gbc->mode) {
		case DMG:
			/*gbc->reg.af = 0x1180u;
			gbc->reg.bc = 0x0000u;
			gbc->reg.de = 0x0008u;
			gbc->reg.hl = 0x007Cu;
			gbc->reg.sp = 0xFFFEu;
			gbc->reg.pc = 0x0100u;*/
			gbc->reg.af = 0x01B0u;
			gbc->reg.bc = 0x0013u;
			gbc->reg.de = 0x00D8u;
			gbc->reg.hl = 0x014Du;
			gbc->reg.sp = 0xFFFEu;
			gbc->reg.pc = 0x0100u;
			break;
		case GBC:
			gbc->reg.af = 0x1180u;
			gbc->reg.bc = 0x0000u;
			gbc->reg.de = 0xFF56u;
			gbc->reg.hl = 0x000Du;
			gbc->reg.sp = 0xFFFEu;
			gbc->reg.pc = 0x0100u;
			break;
	}
}

void gbcc_init_mmap(struct gbc *gbc)
{
	unsigned int wram_mult;
	unsigned int vram_mult;
	
	switch (gbc->mode) {
		case DMG:
			wram_mult = 2;
			vram_mult = 1;
			break;
		case GBC:
			wram_mult = 8;
			vram_mult = 2;
			break;
	}

	gbc->memory.emu_wram = (uint8_t *) calloc(WRAM0_SIZE * wram_mult, 1);
	if (gbc->memory.emu_wram == NULL) {
		gbcc_log(GBCC_LOG_ERROR, "Error allocating WRAM.\n");
		exit(1);
	}

	gbc->memory.emu_vram = (uint8_t *) calloc(VRAM_SIZE * vram_mult, 1);
	if (gbc->memory.emu_vram == NULL) {
		gbcc_log(GBCC_LOG_ERROR, "Error allocating VRAM.\n");
		exit(1);
	}

	gbc->memory.rom0 = gbc->cart.rom;
	gbc->memory.romx = gbc->cart.rom + ROM0_SIZE;
	gbc->memory.vram = gbc->memory.emu_vram;
	gbc->memory.sram = gbc->cart.ram;
	gbc->memory.wram0 = gbc->memory.emu_wram;
	gbc->memory.wramx = gbc->memory.emu_wram + WRAM0_SIZE;
	gbc->memory.echo = gbc->memory.wram0;
}

void gbcc_init_ioreg(struct gbc *gbc)
{
	/* Values taken from the pandocs */
	gbc->memory.ioreg[TIMA - IOREG_START] = 0x00u;
	gbc->memory.ioreg[TMA - IOREG_START] = 0x00u;
	gbc->memory.ioreg[TAC - IOREG_START] = 0x00u;
	gbc->memory.ioreg[NR10 - IOREG_START] = 0x80u;
	gbc->memory.ioreg[NR11 - IOREG_START] = 0xBFu;
	gbc->memory.ioreg[NR12 - IOREG_START] = 0xF3u;
	gbc->memory.ioreg[NR14 - IOREG_START] = 0xBFu;
	gbc->memory.ioreg[NR21 - IOREG_START] = 0x3Fu;
	gbc->memory.ioreg[NR22 - IOREG_START] = 0x00u;
	gbc->memory.ioreg[NR24 - IOREG_START] = 0xBFu;
	gbc->memory.ioreg[NR30 - IOREG_START] = 0x7Fu;
	gbc->memory.ioreg[NR31 - IOREG_START] = 0xFFu;
	gbc->memory.ioreg[NR32 - IOREG_START] = 0x9Fu;
	gbc->memory.ioreg[NR33 - IOREG_START] = 0xBFu;
	gbc->memory.ioreg[NR41 - IOREG_START] = 0xFFu;
	gbc->memory.ioreg[NR42 - IOREG_START] = 0x00u;
	gbc->memory.ioreg[NR43 - IOREG_START] = 0x00u;
	gbc->memory.ioreg[NR30 - IOREG_START] = 0xBFu;
	gbc->memory.ioreg[NR50 - IOREG_START] = 0x77u;
	gbc->memory.ioreg[NR51 - IOREG_START] = 0xF3u;
	gbc->memory.ioreg[NR52 - IOREG_START] = 0xF1u;
	gbc->memory.ioreg[LCDC - IOREG_START] = 0x91u;
	gbc->memory.ioreg[SCY - IOREG_START] = 0x00u;
	gbc->memory.ioreg[SCX - IOREG_START] = 0x00u;
	gbc->memory.ioreg[LYC - IOREG_START] = 0x00u;
	gbc->memory.ioreg[BGP - IOREG_START] = 0xFCu;
	gbc->memory.ioreg[OBP0 - IOREG_START] = 0xFFu;
	gbc->memory.ioreg[OBP1 - IOREG_START] = 0xFFu;
	gbc->memory.ioreg[WY - IOREG_START] = 0x00u;
	gbc->memory.ioreg[WX - IOREG_START] = 0x00u;
	gbc->memory.iereg = 0x00u;
	
	/* Other */
	gbc->memory.ioreg[JOYP - IOREG_START] = 0xFFu;
	gbc->memory.ioreg[LY - IOREG_START] = 0x00u;
}

void gbcc_init_input(struct gbc *gbc)
{
	gbc->keys.a = false;
	gbc->keys.b = false;
	gbc->keys.start = false;
	gbc->keys.select = false;
	gbc->keys.dpad.up = false;
	gbc->keys.dpad.down = false;
	gbc->keys.dpad.left = false;
	gbc->keys.dpad.right = false;
}
