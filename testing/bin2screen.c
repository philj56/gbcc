#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define BUF_LEN 32

void n2c(uint8_t n)
{
	switch (n) {
		case 0:
			printf(" ");
			break;
		case 1:
			printf("░");
			break;
		case 2:
			printf("▒");
			break;
		case 3:
			printf("█");
			break;
	}
}

int main(int argc, char **argv)
{
	if (argc == 1) {
		printf("Usage: %s bin_file\n", argv[0]);
		exit(1);
	}

	uint8_t buf[BUF_LEN];
	FILE *f = fopen(argv[1], "rb");
	while (fread(buf, BUF_LEN, 1, f)) {
		for (uint8_t i = 0; i < BUF_LEN; i++) {
			n2c((buf[i] & 0xC0u) >> 6);
			n2c((buf[i] & 0x30u) >> 4);
			n2c((buf[i] & 0x0Cu) >> 2);
			n2c((buf[i] & 0x03u) >> 0);
			i++;
			n2c((buf[i] & 0xC0u) >> 6);
			n2c((buf[i] & 0x30u) >> 4);
			n2c((buf[i] & 0x0Cu) >> 2);
			n2c((buf[i] & 0x03u) >> 0);
			printf("\n");
		}
	}
	fclose(f);
	return 0;
}
