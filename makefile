ODIR=obj
SRCDIR=src
CC=gcc
CFLAGS=-Wall -Wextra -pedantic -O2 -g
LIBS=-lSDL2 -lm

_DEPS = 
DEPS = $(patsubst %,$(SRCDIR)/%,$(_DEPS))

_OBJ = chip8.o chip8_window.o chip8_input.o chip8_audio.o filters/scale2x.o filters/scale3x.o main.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

chip8: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

$(OBJ): | $(ODIR)

$(ODIR):
	mkdir -p $@
	mkdir -p $@/filters


.PHONY: clean

clean:
	rm -rf $(ODIR) $(SRCDIR)/*~ chip8
