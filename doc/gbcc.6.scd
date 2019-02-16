gbcc(6)

# NAME

gbcc - Game Boy Color emulator written in C

# SYNOPSIS

*gbcc* [-hisvV] [-p _palette_] [-t _speed_] rom_file

# DESCRIPTION

gbcc is a Game Boy Color emulator, created mostly as an exercise in emulator
development and writing a large project in C. Development is ongoing (though
slow).

# OPTIONS

*-h, --help*
	Show help message and exit.

*-i, --interlace*
	Enable interlaced mode (experimental).

*-p, --palette*=_palette_
	Select the color palette for use in DMG mode.

*-s, --subpixel*
	Enable 3x 'subpixel' scaling.

*-t, --turbo*=_speed_
	Set a fractional speed limit for turbo mode. Defaults to 0 (unlimited).

*-v, --vsync*
	Enable VSync.

*-V, --vram-window*
	Display a window containing VRAM tile data.

# FILES

Note: these are the defaults as defined at compile time.

_/usr/share/games/gbcc/tileset.png_
	Tileset used for the on-screen display.

# BUGS

Hahahaha....

# AUTHORS

Philip Jones <philj56@gmail.com>
