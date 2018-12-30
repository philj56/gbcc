ODIR=obj
SRCDIR=src
CC=clang
CFLAGS=-std=c11 -D_POSIX_C_SOURCE=200809L -O2 -flto=full -march=native \
       -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers
LIBS=-lSDL2 -lpng -lm -lpthread

_DEPS = gbcc.h gbcc_apu.h gbcc_audio.h gbcc_constants.h gbcc_cpu.h gbcc_ops.h \
	gbcc_memory.h gbcc_mbc.h gbcc_window.h gbcc_input.h gbcc_bit_utils.h \
	gbcc_debug.h gbcc_ppu.h gbcc_save.h gbcc_colour.h gbcc_hdma.h \
	gbcc_palettes.h gbcc_fontmap.h gbcc_time.h gbcc_vram_window.h \
	gbcc_screenshot.h
DEPS = Makefile $(patsubst %,$(SRCDIR)/%,$(_DEPS))

_OBJ = main.o gbcc_apu.o gbcc_audio.o gbcc_init.o gbcc_cpu.o gbcc_ops.o \
       gbcc_memory.o gbcc_mbc.o gbcc_window.o gbcc_input.o gbcc_bit_utils.o \
       gbcc_debug.o gbcc_ppu.o gbcc_save.o gbcc_colour.o gbcc_hdma.o \
       gbcc_palettes.o gbcc_fontmap.o gbcc_time.o gbcc_vram_window.o \
       gbcc_screenshot.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

gbcc: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

$(OBJ): | $(ODIR)

$(ODIR):
	mkdir -p $@


.PHONY: clean

clean:
	rm -rf $(ODIR) $(SRCDIR)/*~ gbcc
