project(
  'gbcc',
  'c',
  license: 'MIT',
  default_options: [
    'c_std=c11',
    'optimization=3',
    'buildtype=release',
    'warning_level=3',
    'b_lto=true',
    'b_pie=true',
    'prefix=/usr'
  ],
)

if get_option('buildtype').startswith('debug')
  add_project_arguments('-DDEBUG', language : 'c')
endif

data_location = join_paths(
  get_option('prefix'),
  get_option('datadir'),
  'gbcc'
)

tileset_location = join_paths(
  data_location,
  'tileset.png'
)

printer_sound_location = join_paths(
  data_location,
  'print.wav'
)

gtk_ui_location = join_paths(
  data_location,
  'gbcc.ui'
)

shader_location = join_paths(
  data_location,
  'shaders',
  ''
)

completion_location = join_paths(
  get_option('prefix'),
  get_option('datadir'),
  'bash-completion',
  'completions'
)

add_project_arguments(
  [
    '-march=native',
    '-mtune=native',
    '-Wno-unused-parameter',
    '-Wno-missing-braces',
    '-D_POSIX_C_SOURCE=200809L',
    '-D_GNU_SOURCE',
    '-DTILESET_PATH="@0@"'.format(tileset_location),
    '-DPRINTER_SOUND_PATH="@0@"'.format(printer_sound_location),
    '-DGTK_UI_PATH="@0@"'.format(gtk_ui_location),
    '-DSHADER_PATH="@0@"'.format(shader_location),
    #'-fprofile-instr-use',
    #'-fprofile-instr-generate',
  ],
  language: 'c'
)

common_sources = files(
  'src/apu.c',
  'src/args.c',
  'src/audio.c',
  'src/bit_utils.c',
  'src/colour.c',
  'src/config.c',
  'src/cpu.c',
  'src/debug.c',
  'src/fontmap.c',
  'src/gbcc.c',
  'src/hdma.c',
  'src/init.c',
  'src/input.c',
  'src/mbc.c',
  'src/memory.c',
  'src/menu.c',
  'src/ops.c',
  'src/palettes.c',
  'src/ppu.c',
  'src/printer.c',
  'src/save.c',
  'src/screenshot.c',
  'src/time.c',
  'src/wav.c',
  'src/window.c',
  'src/vram_window.c'
)

sdl_sources = files(
  'src/sdl/main.c',
  'src/sdl/sdl.c',
  'src/sdl/vram_window.c',
)

gtk_sources = files(
  'src/gtk/main.c',
  'src/gtk/gtk.c',
  'src/gtk/input.c',
)

cc = meson.get_compiler('c')
sdl = dependency('sdl2')
png = dependency('libpng')
gl = dependency('gl')
epoxy = dependency('epoxy')
openal = dependency('openal')
thread = dependency('threads')
gtk = dependency('gtk+-3.0')

libgbcc = static_library(
  'gbcc',
  common_sources,
  dependencies: [png, gl, epoxy, openal, thread],
  install: false
)

executable(
  'gbcc',
  sdl_sources,
  dependencies: [sdl, thread],
  install: true,
  link_with: libgbcc,
  #link_args: ['-fprofile-instr-use']
  #link_args: ['-fprofile-instr-generate']
)

executable(
  'gbcc-gtk',
  gtk_sources,
  dependencies: [sdl, gtk, thread],
  install: true,
  link_with: libgbcc,
  #link_args: ['-fprofile-instr-use']
  #link_args: ['-fprofile-instr-generate']
)

install_data(
  'tileset.png'
)

install_data(
  'print.wav'
)

install_data(
  'src/gtk/gbcc.ui',
  rename: 'gbcc.ui'
)

install_subdir(
  'shaders',
  install_dir: data_location
)

license_dir = join_paths(
  get_option('datadir'),
  'licenses',
  'gbcc'
)

install_data(
  'LICENSE',
  install_dir: license_dir
)

install_data(
  'completions/gbcc',
  install_dir: completion_location
)

scdoc = find_program('scdoc', required: get_option('man-pages'))
if scdoc.found()
  sed = find_program('sed')
  sh = find_program('sh')
  mandir = get_option('mandir')
  manpage = 'doc/gbcc.6.scd'
  output = 'gbcc.6'
  custom_target(
    output,
    input: manpage,
    output: output,
    command: [
      sh, '-c', '@0@ < @INPUT@ | sed "s|TILESET_PATH|@1@|g; s|SHADER_PATH|@2@|g; s|PRINTER_SOUND_PATH|@3@|g;" > @4@'.format(scdoc.path(), tileset_location, shader_location, printer_sound_location, output)
    ],
    install: true,
    install_dir: '@0@/man6'.format(mandir)
  )
endif
