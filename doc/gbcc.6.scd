gbcc(6)

# NAME

gbcc - Game Boy Color emulator written in C

# SYNOPSIS

*gbcc* [-fhisvV] [-p _palette_] [-t _speed_] rom_file

# DESCRIPTION

gbcc is a Game Boy Color emulator, created mostly as an exercise in emulator
development and writing a large project in C. Development is ongoing (though
slow).

# OPTIONS

*-b, --background*
	Enable playback while unfocused.

*-c, --config*
	Specify path to custom config file.

*-f, --fractional*
	Enable fractional scaling (default is integer).

*-h, --help*
	Show help message and exit.

*-i, --interlace*
	Enable interlaced mode (experimental).

*-p, --palette*=_palette_
	Select the color palette for use in DMG mode.

*-s, --subpixel*
	Enable 7x 'subpixel' scaling by default.

*-t, --turbo*=_speed_
	Set a fractional speed limit for turbo mode. Defaults to 0 (unlimited).

*-v, --vsync*
	Enable VSync.

*-V, --vram-window*
	Display a window containing VRAM tile data.

# KEYS

The following are the keybindings as for a standard QWERTY keyboard, and will
vary for others.

z
	A button

x
	B button

<Return>
	Start

<Space>
	Select

<Arrow keys>
	D-Pad

<Right Shift>
	Turbo mode (toggle)

s
	Filtered & scaled screenshot

S
	Raw screenshot

p
	Pause emulation

f
	Toggle FPS counter

l
	Connect the link cable port to itself. This makes the GameBoy act as if
	it is talking to an exact copy of itself, and enables e.g. trading
	(and battling!) with oneself in Pokémon.

b
	Toggle background playback.

v
	Cycle through available shaders

V
	Toggle VRAM window display.

1
	Toggle background display

2
	Toggle window display

3
	Toggle sprites display

<F0-9>
	Load savestate 0-9

<Left Shift + F0-9>
	Save savestate 0-9

# FILES

Note: these are the defaults as defined at compile time.

_TILESET_PATH_
	Tileset used for the on-screen display.

_SHADER_PATH_
	Folder containing all GLSL shaders used for rendering.

$XDG_CONFIG_HOME/gbcc/config.
	Config file.

# SAVE FILES

Saves are created with the same name as the rom, ending in .sav. The save file
format is a plain dump of SRAM. For MBC3 cartridges, RTC data is appended to
this ram dump in plain text.

# BUGS

Surprisingly few.

# AUTHORS

Philip Jones <philj56@gmail.com>
