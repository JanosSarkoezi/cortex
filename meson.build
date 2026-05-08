project('cortex', 'c',
  version : '0.1.0',
  default_options : ['warning_level=3', 'c_std=c11'])

# Abhängigkeiten definieren
# Wir suchen zuerst im System, falls nicht vorhanden, nutzen wir subprojects
glfw_dep = dependency('glfw3', fallback : ['glfw', 'glfw_dep'])
cglm_dep = dependency('cglm', fallback : ['cglm', 'cglm_dep'])

# OpenGL ist meist im System vorhanden
gl_dep = dependency('gl')

# Falls du GLAD als Loader nutzt, füge hier die Quelldatei hinzu
# (Den GLAD-Webgenerator nutzt man meist vorab)
inc = include_directories('include')

sources = [
  'src/main.c',
  'src/shader_loader.c' # Falls du die Shader-Logik auslagerst
]

executable('cortex',
  sources,
  dependencies : [glfw_dep, cglm_dep, gl_dep],
  include_directories : inc,
  install : true)
