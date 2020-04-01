default: all

all: intro.gbc

intro.gbc: intro.o
	rgblink -o $@ $^
	rgbfix -v -p0 $@

intro.o: intro.rgbasm font.rgbasm
	rgbasm -o $@ $<

clean:
	rm -f intro.o intro.gbc
