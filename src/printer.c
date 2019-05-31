#include "bit_utils.h"
#include "debug.h"
#include "printer.h"
#include <stdio.h>
#include <string.h>

/* Magic bytes that mark the start of a command packet */
#define MAGIC_BYTE_1 0x88u
#define MAGIC_BYTE_2 0x33u

#define PRINTER_WIDTH_TILES 20

/* Meaning of command bytes */
#define INITALISE 0x01u
#define START_PRINTING 0x02u
#define FILL_BUFFER 0x04u
#define READ_STATUS 0x0Fu

/* Order of bytes in packet, ignoring length of data */
#define COMMAND 0
#define COMPRESSION 1
#define LENGTH_LSB 2
#define LENGTH_MSB 3
#define DATA 4
#define CHECKSUM_LSB 5
#define CHECKSUM_MSB 6
#define ALIVE_INDICATOR 7
#define STATUS 8

static void check_magic(struct printer *p, uint8_t byte);
static void execute(struct printer *p);
static void initialise(struct printer *p);
static void start_printing(struct printer *p);
static void fill_buffer(struct printer *p, uint8_t byte);
static void read_status(struct printer *p);
static void parse_print_args(struct printer *p, uint8_t byte);
static uint8_t get_palette_colour(struct printer *p, uint8_t colour);

uint8_t gbcc_printer_parse_byte(struct printer *p, uint8_t byte)
{
	if (!p->in_packet) {
		check_magic(p, byte);
		return 0;
	}
	switch(p->packet.current_byte) {
		case COMMAND:
			p->packet.command = byte;
			p->packet.current_byte++;
			p->packet.printer_checksum += byte;
			break;
		case COMPRESSION:
			/* TODO: Handle compression */
			p->packet.compression = byte;
			p->packet.current_byte++;
			p->packet.printer_checksum += byte;
			break;
		case LENGTH_LSB:
			p->packet.data_length = byte;
			p->packet.current_byte++;
			p->packet.printer_checksum += byte;
			break;
		case LENGTH_MSB:
			p->packet.data_length |= byte << 8u;
			p->packet.current_byte++;
			p->packet.printer_checksum += byte;
			break;
		case DATA:
			if (p->packet.data_byte < p->packet.data_length) {
				if (p->packet.command == START_PRINTING) {
					parse_print_args(p, byte);
				} else if (p->packet.command == FILL_BUFFER) {
					fill_buffer(p, byte);
				}
				p->packet.data_byte++;
				p->packet.printer_checksum += byte;
				break;
			}
			p->packet.current_byte++;
			/* Fallthrough */
		case CHECKSUM_LSB:
			p->packet.gb_checksum = byte;
			p->packet.current_byte++;
			break;
		case CHECKSUM_MSB:
			p->packet.gb_checksum |= byte << 8u;
			p->packet.current_byte++;
			break;
		case ALIVE_INDICATOR:
			p->packet.current_byte++;
			return 0x81u;
		case STATUS:
			p->magic = 0;
			p->in_packet = 0;
			if (p->packet.gb_checksum != p->packet.printer_checksum) {
				gbcc_log_error("Printer checksum incorrect: 0x%04X vs 0x%04X.\n", p->packet.printer_checksum, p->packet.gb_checksum);
				p->status = set_bit(p->status, 0);
			}
			execute(p);
			p->packet = (struct packet){0};
			return p->status;
	}
	return 0;
}

void check_magic(struct printer *p, uint8_t byte)
{
	switch (byte) {
		case MAGIC_BYTE_1:
			p->magic = 1;
			break;
		case MAGIC_BYTE_2:
			if (p->magic) {
				p->in_packet = true;
			}
			/* Fallthrough */
		default:
			p->magic = 0;
	}
}

void execute(struct printer *p)
{
	switch (p->packet.command) {
		case 0x1u:
			initialise(p);
			break;
		case 0x2u:
			start_printing(p);
			break;
		case 0x4u:
			break;
		case 0xFu:
			read_status(p);
			break;
		default:
			gbcc_log_error("Invalid printer command %02X.\n", p->packet.command);
			break;
	}
}

void initialise(struct printer *p)
{
	memset(p->image_buffer.data, 0, sizeof(p->image_buffer.data));
	p->image_buffer.length = 0;
	p->status = 0;
}

void start_printing(struct printer *p)
{
	uint16_t byte = 0;
	for (int line = 0; byte < p->image_buffer.length; line++) {
		uint8_t ty = line / 8;
		for (uint8_t tx = 0; tx < PRINTER_WIDTH_TILES; tx++) {
			uint16_t idx = ty * PRINTER_WIDTH_TILES * 16 + tx * 16 + (line - ty * 8) * 2;
			uint8_t lo = p->image_buffer.data[idx];
			uint8_t hi = p->image_buffer.data[idx + 1];
			for (uint8_t x = 0; x < 8; x++) {
				switch (get_palette_colour(p, (check_bit(hi, 7 - x) << 1u) | check_bit(lo, 7 - x))) {
					case 0:
						printf("█");
						break;
					case 1:
						printf("▒");
						break;
					case 2:
						printf("░");
						break;
					case 3:
						printf(" ");
						break;
				}
			}
			byte += 2;
		}
		printf("\n");
	}
	p->image_buffer.length = 0;
}

uint8_t get_palette_colour(struct printer *p, uint8_t colour) {
	uint8_t colours[4] = {
		(p->palette & 0x03u) >> 0u,
		(p->palette & 0x0Cu) >> 2u,
		(p->palette & 0x30u) >> 4u,
		(p->palette & 0xC0u) >> 6u
	};
	return colours[colour];
}

void fill_buffer(struct printer *p, uint8_t byte)
{
	if (p->image_buffer.length <= GBC_PRINTER_IMAGE_BUFFER_SIZE) {
		p->image_buffer.data[p->image_buffer.length] = byte;
		p->image_buffer.length++;
	} else {
		p->status = set_bit(p->status, 2);
	}
}

void read_status(struct printer *p)
{
}

void parse_print_args(struct printer *p, uint8_t byte)
{
	switch (p->packet.data_byte) {
		case 0:
			/* TODO */
			break;
		case 1:
			p->margins = byte;
			break;
		case 2:
			p->palette = byte;
			break;
		case 3:
			p->exposure = byte;
			break;
	}
}
