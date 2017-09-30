#include <stdio.h>
#include "gbc.h"

int main()
{
	struct gbc gbc;
	gbc.reg.a = 1;
	printf("%u\n", gbc.reg.a);
	return 0;
}
