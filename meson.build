project('cortex', 'c',
  version : '0.1.0',
  default_options : ['warning_level=3', 'c_std=c11'])

cc = meson.get_compiler('c')

# Abhängigkeiten definieren
glfw_dep = dependency('glfw3', fallback : ['glfw', 'glfw_dep'])
cglm_dep = dependency('cglm', fallback : ['cglm', 'cglm_dep'])
gl_dep = dependency('gl')
dl_dep = cc.find_library('dl', required : false)

inc = include_directories('include')

sources = [
  'src/main.c',
  'src/glad.c',
  'src/shader_loader.c',
  'src/shader_utils.c',
  'src/file_utils.c',
  'src/camera.c'
]

executable('cortex',
  sources,
  dependencies : [glfw_dep, cglm_dep, gl_dep, dl_dep],
  include_directories : inc,
  install : true)
