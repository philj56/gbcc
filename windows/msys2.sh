#!/bin/bash -e

USE_CLANG=true
INSTALL_DEPS=false

usage() { echo "Usage: $0 [--install-deps] [--no-clang]" 1>&2; exit 1; }


# read arguments
opts="$(getopt \
	--longoptions "install-deps,no-clang,help" \
	--name "$(basename "$0")" \
	--options "h" \
	-- "$@"
)"

eval set -- "$opts"
unset opts

while true; do
	case $1 in
		--install-deps)
			INSTALL_DEPS=true
			shift 1
			;;

		--no-clang)
			USE_CLANG=false
			shift 1
			;;

		-h|--help)
			usage
			;;

		*)
			break
			;;
	esac
done

if $INSTALL_DEPS; then
	echo | pacman -S --needed --noconfirm\
		mingw-w64-x86_64-toolchain\
		mingw-w64-x86_64-clang\
		mingw-w64-x86_64-lld\
		mingw-w64-x86_64-meson\
		mingw-w64-x86_64-SDL2\
		mingw-w64-x86_64-libpng\
		mingw-w64-x86_64-libepoxy\
		mingw-w64-x86_64-openal
fi

if [ ! -d "build" ]; then
	if $USE_CLANG; then
		CC=clang LD=lld LDSHARED="clang -shared" meson .. build
	else
		meson .. build
	fi
	meson configure build -Db_pie=false
fi

# Meson sets -Wl,--allow-shlib-undefined, which isn't recognised by the version
# of lld used by MSYS2. In addition, build.ninja gets regenerated every time
# something changes, so we just allow ninja to fail once here, remove the
# argument, and re-run the build
ninja -C build || sed -i 's/"-Wl,--allow-shlib-undefined"//' build/build.ninja && ninja -C build

mkdir -p dist

files=(
	"build/gbcc"
	"../tileset.png"
	"../print.wav"
	"../LICENSE"
	"../shaders"
)
files+=($(ldd build/gbcc | grep mingw | sed "s/.*=> \([^(]\+\).*/\1/g"))

for f in ${files[@]}; do
	cp -r $f dist
done

cat license_additions.txt >> dist/LICENSE
mv dist/LICENSE{,.txt}

echo "\nBuild complete, files are in ./dist"
