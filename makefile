ODIR=obj
SRCDIR=src
CC=gcc
CFLAGS=-Wall -Wextra -pedantic -g
LIBS=-lSDL2 -lm

_DEPS = gbcc.h gbcc_constants.h
DEPS = $(patsubst %,$(SRCDIR)/%,$(_DEPS))

_OBJ = main.o gbcc_init.o gbcc_cpu.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

gbcc: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

$(OBJ): | $(ODIR)

$(ODIR):
	mkdir -p $@


.PHONY: clean

clean:
	rm -rf $(ODIR) $(SRCDIR)/*~ gbcc
