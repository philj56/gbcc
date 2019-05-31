#ifndef GBCC_PRINTER_H
#define GBCC_PRINTER_H

#include <stdbool.h>
#include <stdint.h>

#define GBC_PRINTER_IMAGE_BUFFER_SIZE 0x2000

struct printer {
	struct {
		uint8_t data[GBC_PRINTER_IMAGE_BUFFER_SIZE];
		uint16_t length;
	} image_buffer;
	struct packet {
		uint8_t command;
		uint8_t compression;
		uint16_t data_length;
		uint16_t printer_checksum;
		uint16_t gb_checksum;
		uint16_t current_byte; 	/* Which byte are we about to read? */
		uint16_t data_byte;
	} packet;
	uint8_t margins;
	uint8_t palette;
	uint8_t exposure;
	uint8_t status;
	bool magic; 	/* Flag for detecting magic bytes that start a packet */
	bool in_packet;
	bool connected;
};

uint8_t gbcc_printer_parse_byte(struct printer *p, uint8_t byte);

#endif /* GBCC_PRINTER_H */
