project(
  'gbcc',
  'c',
  license: 'MIT',
  default_options: [
    'c_std=c11',
  ],
)

add_project_arguments(
  [
    '-D_POSIX_C_SOURCE=200809L',
    '-O2',
    '-flto',
    '-march=native',
    '-Wall',
    '-Wextra',
    '-Wno-unused-parameter',
    '-Wno-missing-field-initializers',
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

sdl = dependency('sdl2')
png = dependency('libpng')

executable(
  'gbcc',
  sources,
  dependencies: [sdl, png],
  link_args: ['-flto', '-lm'],
  install: true,
)

install_data(
  'tileset.png'
)

scdoc = find_program('scdoc', required: get_option('man-pages'))
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
      sh, '-c', '@0@ < @INPUT@ > @1@'.format(scdoc.path(), output)
    ],
    install: true,
    install_dir: '@0@/man6'.format(mandir)
  )
endif
