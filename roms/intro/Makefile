default: all

all: intro.gbc

intro.gbc: intro.o bounce.o palettes.o
	rgblink -o $@ $^
	rgbfix -v -p0 $@

intro.o: intro.rgbasm font.rgbasm
	rgbasm -o $@ $<

bounce.o: bounce.rgbasm
	rgbasm -o $@ $<

palettes.o: palettes.rgbasm
	rgbasm -o $@ $<

clean:
	rm -f intro.o intro.gbc
