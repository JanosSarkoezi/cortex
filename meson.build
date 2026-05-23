project('cortex', 'c',
  version : '0.1.0',
  default_options : ['warning_level=3', 'c_std=c11', 'default_library=static'])

cc = meson.get_compiler('c')
python = import('python').find_installation('python3')

# --- 1. Shader & Binary Embedding Automatisierung ---
# Dieses Target generiert src/shaders_embedded.h und .c neu, wenn sich die Dateien in shaders/ ändern
shaders_gen = custom_target('shader_embedding',
  output : ['shaders_embedded.h', 'shaders_embedded.c'],
  command : [python, files('scripts/embed_shaders.py')],
  build_by_default : true
)

# Target für Font-Einbettung
font_gen = custom_target('font_embedding',
  input : 'fonts/JetBrainsMono-Regular.ttf',
  output : ['font_embedded.h', 'font_embedded.c'],
  command : [python, files('scripts/embed_binary.py'), '@INPUT@', 'font_data', '@OUTPUT0@', '@OUTPUT1@'],
  build_by_default : true
)

# --- 2. Abhängigkeiten (Statisch erzwingen) ---
# cglm: als Subprojekt einbinden, Installation explizit deaktivieren
glfw_dep = dependency('glfw3', static : true, fallback : ['glfw', 'glfw_dep'])
cglm_dep = dependency('cglm',
  static  : true,
  fallback : ['cglm', 'cglm_dep'],
  default_options : ['install=false']
)

# OpenGL muss unter Linux fast immer dynamisch bleiben
gl_dep = dependency('gl')
dl_dep = cc.find_library('dl', required : false)

inc = include_directories('include', 'src')

# Trenne glad.c von den restlichen Quellen
glad_res = static_library('glad', 'src/glad.c',
  include_directories : include_directories('include'),
  c_args : ['-w']
)

sources = [
  'src/main.c',
  'src/shader_loader.c',
  'src/shader_utils.c',
  'src/file_utils.c',
  'src/camera.c',
  'src/histogram.c',
  'src/text_renderer.c'
]

# --- 3. Executable erstellen ---
executable('cortex',
  sources,
  [shaders_gen, font_gen],
  link_with : glad_res,
  dependencies : [glfw_dep, cglm_dep, gl_dep, dl_dep],
  include_directories : inc,
  install : false,
  link_args : ['-static-libgcc']
)
