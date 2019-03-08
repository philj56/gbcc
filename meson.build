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

data_location = join_paths(
  get_option('prefix'),
  get_option('datadir'),
  'gbcc'
)

tileset_location = join_paths(
  data_location,
  'tileset.png'
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
    '-Wno-missing-field-initializers',
    '-DTILESET_PATH="@0@"'.format(tileset_location),
    '-DSHADER_PATH="@0@"'.format(shader_location),
  ],
  language: 'c'
)

sources = files(
  'src/main.c',
  'src/gbcc_apu.c',
  'src/gbcc_audio.c',
  'src/gbcc_init.c',
  'src/gbcc_cpu.c',
  'src/gbcc_ops.c',
  'src/gbcc_memory.c',
  'src/gbcc_mbc.c',
  'src/gbcc_window.c',
  'src/gbcc_input.c',
  'src/gbcc_bit_utils.c',
  'src/gbcc_debug.c',
  'src/gbcc_ppu.c',
  'src/gbcc_save.c',
  'src/gbcc_colour.c',
  'src/gbcc_hdma.c',
  'src/gbcc_palettes.c',
  'src/gbcc_fontmap.c',
  'src/gbcc_time.c',
  'src/gbcc_vram_window.c',
  'src/gbcc_screenshot.c',
)

cc = meson.get_compiler('c')
m = cc.find_library('m', required: false)
sdl = dependency('sdl2')
png = dependency('libpng')
gl = dependency('gl')
glew = dependency('glew')


executable(
  'gbcc',
  sources,
  dependencies: [m, sdl, png, gl, glew],
  install: true,
  link_args: []
)

install_data(
  'tileset.png'
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
sed = find_program('sed', required: get_option('man-pages'))
if scdoc.found()
  sh = find_program('sh')
  mandir = get_option('mandir')
  manpage = 'doc/gbcc.6.scd'
  output = 'gbcc.6'
  custom_target(
    output,
    input: manpage,
    output: output,
    command: [
      sh, '-c', '@0@ < @INPUT@ | sed "s|TILESET_PATH|@1@|g; s|SHADER_PATH|@2@|g" > @3@'.format(scdoc.path(), tileset_location, shader_location, output)
    ],
    install: true,
    install_dir: '@0@/man6'.format(mandir)
  )
endif
