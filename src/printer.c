/*
 * Copyright (C) 2017-2020 Philip Jones
 *
 * Licensed under the MIT License.
 * See either the LICENSE file, or:
 *
 * https://opensource.org/licenses/MIT
 *
 */

#include "bit_utils.h"
#include "debug.h"
#include "printer.h"
#include "audio.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifndef PRINTER_SOUND_PATH
#define PRINTER_SOUND_PATH "print.wav"
#endif

/* Magic bytes that mark the start of a command packet */
#define MAGIC_BYTE_1 0x88u
#define MAGIC_BYTE_2 0x33u

#define PRINTER_WIDTH_TILES 20
#define PRINTER_STRIP_HEIGHT 16

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

/* Meaning of status flag bits */
#define STATUS_CHECKSUM_ERROR 0
#define STATUS_PRINTING 1
#define STATUS_DATA_FULL 2
#define STATUS_DATA_UNPROCESSED 3
#define STATUS_PACKET_ERROR 4
#define STATUS_PAPER_JAM 5
#define STATUS_HEAT_ERROR 6
#define STATUS_LOW_BATTERY 7

static void check_magic(struct printer *p, uint8_t byte);
static void execute(struct printer *p);
static void initialise(struct printer *p);
static void start_printing(struct printer *p);
static void fill_buffer(struct printer *p, uint8_t byte);
static void parse_print_args(struct printer *p, uint8_t byte);
static void *print(void *printer);
static bool print_margin(struct printer *p, bool top);
static bool print_strip(struct printer *p);
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
				switch (p->packet.command) {
					case INITALISE:
						/*
						 * INITIALISE has unknown
						 * function, but seems to have
						 * no effect
						 */
						break;
					case START_PRINTING:
						parse_print_args(p, byte);
						break;
					case FILL_BUFFER:
						fill_buffer(p, byte);
						break;
					case READ_STATUS:
						/* READ_STATUS is a no-op */
						break;
				}
				p->packet.data_byte++;
				p->packet.printer_checksum += byte;
				p->status = set_bit(p->status, STATUS_DATA_UNPROCESSED);
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
				gbcc_log_error("Printer checksum incorrect "
						"(Expected 0x%04X, "
						"Calculated 0x%04X).\n",
						p->packet.printer_checksum,
						p->packet.gb_checksum);
				p->status = set_bit(p->status, STATUS_CHECKSUM_ERROR);
			}
			{
				uint8_t tmp = p->status;
				execute(p);
				p->packet = (struct packet){0};
				return tmp;
			}
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
			if (check_bit(p->status, STATUS_PRINTING)) {
				return;
			}
			initialise(p);
			break;
		case 0x2u:
			if (check_bit(p->status, STATUS_PRINTING)) {
				return;
			}
			start_printing(p);
			break;
		case 0x4u:
			break;
		case 0xFu:
			break;
		default:
			gbcc_log_error("Invalid printer command %02X.\n", p->packet.command);
			break;
	}
}

void initialise(struct printer *p)
{
	*p = (struct printer){0};
}

void start_printing(struct printer *p)
{
	p->status = set_bit(p->status, STATUS_PRINTING);
	pthread_create(&p->print_thread, NULL, print, p);
	pthread_setname_np(p->print_thread, "PrinterThread");
	return;
}

bool print_margin(struct printer *p, bool top) {
	if (top && p->margin.top_line >= p->margin.top_width) {
		return 1;
	} else if (!top && p->margin.bottom_line >= p->margin.bottom_width) {
		return 1;
	}
	for (int line = 0; line < PRINTER_STRIP_HEIGHT; line++) {
		for (uint8_t tx = 0; tx < PRINTER_WIDTH_TILES; tx++) {
			for (uint8_t x = 0; x < 8; x++) {
				printf("█");
			}
		}
		const struct timespec to_sleep = {.tv_sec = 0, .tv_nsec = 3000000};
		nanosleep(&to_sleep, NULL);
		printf("\n");
	}
	if (top) {
		p->margin.top_line++;
	} else {
		p->margin.bottom_line++;
	}
	return 0;
}

bool print_strip(struct printer *p)
{
	if (p->print_byte == p->image_buffer.length) {
		return 1;
	}
	unsigned int line;
	for (line = 0; (line < PRINTER_STRIP_HEIGHT) && (p->print_byte < p->image_buffer.length); line++) {
		uint8_t ty = (uint8_t)((line + p->print_line) / 8);
		for (uint8_t tx = 0; tx < PRINTER_WIDTH_TILES; tx++) {
			uint16_t idx = ty * PRINTER_WIDTH_TILES * 16 + tx * 16 + (uint8_t)(line + p->print_line - ty * 8) * 2;
			uint8_t lo = p->image_buffer.data[idx];
			uint8_t hi = p->image_buffer.data[idx + 1];
			for (uint8_t x = 0; x < 8; x++) {
				switch (get_palette_colour(p, (uint8_t)(check_bit(hi, 7 - x) << 1u) | check_bit(lo, 7 - x))) {
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
			p->print_byte += 2;
		}
		printf("\n");
		const struct timespec to_sleep = {.tv_sec = 0, .tv_nsec = 3000000};
		nanosleep(&to_sleep, NULL);
	}
	p->print_line += line;
	return 0;
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
	if (p->image_buffer.length < GBC_PRINTER_IMAGE_BUFFER_SIZE) {
		p->image_buffer.data[p->image_buffer.length] = byte;
		p->image_buffer.length++;
	} else {
		p->status = set_bit(p->status, STATUS_DATA_FULL);
	}
}

void parse_print_args(struct printer *p, uint8_t byte)
{
	switch (p->packet.data_byte) {
		case 0:
			/* TODO */
			break;
		case 1:
			p->margin.top_width = byte >> 4u;
			p->margin.bottom_width = byte & 0x0Fu;
			break;
		case 2:
			p->palette = byte;
			break;
		case 3:
			p->exposure = byte;
			break;
	}
}

void *print(void *printer)
{
	struct printer *p = (struct printer *)printer;
	int stage = 0;
	while (stage < 3) {
		gbcc_audio_play_wav(PRINTER_SOUND_PATH);
		const struct timespec to_sleep = {.tv_sec = 0, .tv_nsec = 850000000};
		nanosleep(&to_sleep, NULL);
		if (stage == 0) {
			stage += print_margin(p, true);
		}
		if (stage == 1) {
			stage += print_strip(p);
			if (p->print_byte == p->image_buffer.length && p->margin.bottom_width == 0) {
				break;
			}
		}
		if (stage == 2) {
			stage += print_margin(p, false);
			if (p->margin.bottom_line >= p->margin.bottom_width) {
				break;
			}
		}
	}
	initialise(p);
	return 0;
}
