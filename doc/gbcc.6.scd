gbcc(6)

# NAME

gbcc - Game Boy Color emulator written in C

# SYNOPSIS

*gbcc* [-aAbfFhivV] [-c _config_file_] [-p _palette_] [-s _shader_]\
[-t _speed_] rom

# DESCRIPTION

gbcc is a Game Boy & Game Boy Color emulator, created mostly as an exercise in
emulator development and writing a large project in C. Development is ongoing
(though slow).

# OPTIONS

*-a, --autoresume*
	Automatically resume from when gbcc was last closed for this game.

*-A, --autosave*
	Automatically save SRAM after the last write was detected. Default is to save
	on normal program exit.

*-b, --background*
	Enable playback while unfocused.

*-c, --config*
	Specify path to custom config file.

*-f, --fractional*
	Enable fractional scaling (default is integer).

*-F, --frame-blending*
	Enable frame blending.

*-h, --help*
	Show help message and exit.

*-i, --interlace*
	Enable interlaced mode. The Game Boy draws every other line darker, leading
	to interesting visual effects in some games. Using this without
	frame-blending *will* look terrible.

*-p, --palette*=_palette_
	Select the color palette for use in DMG mode.

*-s, --shader*
	Select the shader to use on startup.

*-t, --turbo*=_speed_
	Set a fractional speed limit for turbo mode. Defaults to 0 (unlimited). Audio
	will be disabled while turboing, unless a speed limit is set.

*-v, --vsync*
	Enable Vsync, experimental. By default, gbcc will sync to audio, playing back
	at real Game Boy speed. This leads to slight visual flickering, which is only
	noticeable if playing a game that makes use of transparency. Enabling Vsync
	will cause gbcc to sync to video instead, playing back slightly faster than a
	real Game Boy (by about 0.5%) and eliminating visual flickering.

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

<Escape>
	Toggle settings menu

s
	Filtered & scaled screenshot

<Left Shift> + S
	Raw screenshot

p
	Pause emulation

<Left Shift> + P
	Connect the Gameboy Printer, printing to stdout

f
	Toggle FPS counter

<Left Shift> + F
	Toggle frame blending

l
	Connect the link cable port to itself. This makes the GameBoy act as if
	it is talking to an exact copy of itself, and enables e.g. trading
	(and battling!) with oneself in Pokémon.

<Left Shift> + A
	Toggle autosave

b
	Toggle background playback

v
	Toggle Vsync

<Left Shift> + V
	Toggle VRAM window display

o
	Cycle through available shaders

1
	Toggle background display

2
	Toggle window display

3
	Toggle sprites display

<F1-9>
	Load savestate 1-9

<Left Shift> + <F1-9>
	Save savestate 1-9

<Numpad 2,4,6,8>
	Accelerometer control (MBC7 games only)

<Left Shift> + <Numpad 2,4,6,8>
	Strong accelerometer control (MBC7 games only)

# FILES

Note: these are the defaults as defined at compile time.

_TILESET_PATH_
	Tileset used for the on-screen display.

_PRINTER_SOUND_PATH_
	Sound played while printing.

_SHADER_PATH_
	Folder containing all GLSL shaders used for rendering.

_ICON_PATH_
	Folder containing application icons.

_$XDG_CONFIG_HOME/gbcc/config_
	Config file. Available options are the same as command line options
	(where possible), defined with syntax:

		option = value
	
	For command line equivalents which take no arguments, value is taken to
	be a boolean (i.e. value = 0 disables the option, any other number
	enables it).
.
	Lines starting with '#' are ignored.

	Command line options override config file options.

# SAVE FILES

Saves are created with the same name as the rom, ending in .sav. The save file
format is a plain dump of SRAM. For MBC3 cartridges, RTC data is appended to
this ram dump in plain text.

# SAVE STATES

Save states are created with the same name as the rom, ending in .s[0-9].
Extensions s1-s9 are the save states associated with the corresponding function
keys, while s0 is a backup created every time gbcc is closed. The save state
format is a pure binary dump of the main gbcc struct, so will almost certainly
not work if moved between different architectures / compilers etc.

# BUGS

Surprisingly few.

# AUTHORS

Philip Jones <philj56@gmail.com>
