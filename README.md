# [GBCC](https://gbcc.github.io)
GBCC is a cross-platform Game Boy and Game Boy Color emulator written in C,
with a focus on accuracy. See the [website](https://gbcc.github.io)
or the [manpage](https://gbcc.github.io/manpage) for details.

## Table of Contents
* [Install](#install)
  * [Prebuilt packages](#prebuilt-packages-)
  * [Linux](#linux)
    * [Arch](#arch)
  * [macOS](#macos)
  * [Windows](#windows)
  * [Android](#android)

## Install
### Prebuilt packages ![](https://github.com/philj56/gbcc/workflows/Build%20packages/badge.svg)
Packages for Windows and Ubuntu 18.04 are generated on each commit. To download
them, navigate to the [actions](https://github.com/philj56/gbcc/actions) tab
(you'll need to be logged in to GitHub to do this). Select the latest "Build
Packages" job that succeeded, and look for the artifacts dropdown.

Note that the GTK3 GUI is disabled on the prebuilt Windows packages, as the
necessary .DLLs and icon themes increase the size dramatically, to about 25MiB
(compared to about 2MiB for the SDL2 version). If you want the GTK3 version on
Windows, either download a proper release (whenever I make one), or [build it
yourself](#windows) (I've made it dead easy, honest!).

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
# With clang:
CC=clang LDSHARED="clang -shared" meson build && ninja -C build

# Or without clang:
meson build && ninja -C build
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
# With clang:
export CC=clang LDSHARED="clang -shared"
yay -S gbcc-git

# Or without clang:
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
Download and install MSYS2. The first time you run it after install, you'll
probably have to update, so in a MinGW64 prompt run:
```sh
pacman -Syu
```
This will do some stuff, then prompt you to restart MSYS2 and update again.

After you've updated, building GBCC is easy:
```sh
git clone https://github.com/philj56/gbcc.git
cd gbcc/windows
./msys2.sh --install-deps

# After the first build, --install-deps is not required.
```

This will place the SDL2 build & necessary .DLLs in a folder called `dist`,
which you can then copy wherever you like. As with macOS, the GTK3 GUI is
disabled by default, although not for performance reasons (see [Prebuilt
packages](#prebuilt-packages-)). It can be enabled by passing `--with-gtk` to
the `msys2.sh` script mentioned above.

### Android
You can get GBCC on (Google Play)[https://play.google.com/store/apps/details?id=com.philj56.gbcc],
or from the (project repo)[https://github.com/philj56/gbcc-android/].
