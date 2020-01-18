# [GBCC](https://philj56.github.io/gbcc/)
GBCC is a cross-platform Game Boy and Game Boy Color emulator written in C,
with a focus on accuracy. See the [website](https://philj56.github.io/gbcc/)
or the [manpage](https://philj56.github.io/gbcc/manpage.html) for details.

## Table of Contents
* [Install](#Install)
  * [Linux](#Linux)
    * [Arch](#Arch)
  * [macOS](#macOS)
  * [Windows](#Windows)
  * [Android](#Android)

## Install
### Linux
Install the necessary dependencies, e.g. for Ubuntu:
```sh
sudo apt-get install libsdl2-dev libpng-dev libepoxy-dev libopenal-dev

# Optional dependencies
sudo apt-get install clang  # Preferred compiler
sudo apt-get install meson  # Preferred build system
sudo apt-get install libgtk-3-dev  # For the GTK3 GUI
sudo apt-get install scdoc  # For the manpage
```

To build with meson:
```sh
meson build && ninja -C build

# Or with clang:
CC=clang LDSHARED="clang -shared" meson build && ninja -C build
```

Or to build without:
```sh
clang -o gbcc -O3 -flto src/*.c src/sdl/*.c \
        -lepoxy -lpng -lSDL2 -lpthread -lopenal

clang -o gbcc-gtk -O3 -flto src/*.c src/gtk/*.c \
        -lepoxy -lpng -lSDL2 -lpthread -lopenal \
        $(pkg-config --cflags --libs gtk+-3.0)
```

Building with clang is highly recommended, as it currently produces a binary
about twice as fast as gcc.

#### Arch
GBCC is available on the [AUR](https://aur.archlinux.org/packages/gbcc-git/):
```sh
yay -S gbcc-git

# Or with clang:
export CC=clang LDSHARED="clang -shared"
yay -S gbcc-git
```
As above, building with clang is highly recommended.

### macOS
GBCC is available as a head-only [brew](https://brew.sh/)
[formula](https://github.com/philj56/homebrew-gbcc).
You can build & install it with
```sh
brew install --HEAD philj56/gbcc/gbcc
```

Note that the GTK3 GUI is disabled by default, as OpenGL under GTK on macOS has
pretty appalling performance at the time of writing. It can be enabled by
passing `--with-gtk+3` to the above command.

### Windows
Support for building on windows is provided by [MSYS2](https://www.msys2.org/).
In a MinGW64 prompt:
```sh
git clone https://github.com/philj56/gbcc.git
cd gbcc/windows
./msys2.sh --install-deps

# After the first build, --install-deps is not required.
```

This will place the SDL2 build & necessary .DLLs in a folder called `dist`.

### Android
Coming soon™
