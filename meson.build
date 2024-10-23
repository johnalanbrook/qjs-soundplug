project('qjs-soundplug', 'c')

inc = include_directories('.')

add_project_arguments('-Wno-incompatible-function-pointer-types', language: 'c')
add_project_arguments('-Wno-incompatible-pointer-types', language: 'c')

libtype = get_option('default_library')
if libtype == 'shared'
  add_project_arguments('-DJS_SHARED_LIBRARY', language:'c')
endif

sources = [
  'iir.c',
  'dsp.c',
  'music.c',
  'sound.c',
]

quickjs = dependency('quickjs', fallback:['quickjs', 'quickjs_dep'])

libtype = get_option('default_library')
if libtype == 'static'
  add_project_arguments('-DJS_SHARED_LIBRARY', language:'c')
endif

qjs_soundplug_lib = library('qjs-soundplug',
  sources: sources + 'qjs_soundplug.c',
  include_directories: [inc],
  dependencies: [quickjs],
)

qjs_soundplug_dep = declare_dependency(
  link_with: qjs_soundplug_lib,
  include_directories : inc
)

qjs_sokol_audio_lib = library('qjs-sokol_audio',
  sources: 'qjs_sokol_audio.c',
  include_directories: inc,
  dependencies: [quickjs]
)