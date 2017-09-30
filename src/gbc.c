

void gbc_load_rom(struct gbc* gbc, const char *filename)
{
	size_t read;
	FILE *rom = fopen(filename, "r");
	if (rom == NULL)
	{
		fprintf(stderr, "Error opening file: %s\n", filename);
		exit(1);
	}
	read = fread(&(chip->memory[CHIP8_PROGRAM_START]), 1, CHIP8_MEMORY_SIZE - CHIP8_PROGRAM_START, rom);
	fclose(rom);
	if (read == 0) {
		fprintf(stderr, "Error reading from file: %s\n", filename);
		exit(1);
	}
}
