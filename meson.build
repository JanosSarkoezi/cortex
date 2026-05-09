project('cortex', 'c',
  version : '0.1.0',
  default_options : ['warning_level=3', 'c_std=c11', 'default_library=static'])

cc = meson.get_compiler('c')

# --- 1. Abhängigkeiten (Statisch erzwingen) ---
# Durch 'static : true' sucht Meson nach .a Dateien statt .so
glfw_dep = dependency('glfw3', static : true, fallback : ['glfw', 'glfw_dep'])
cglm_dep = dependency('cglm', static : true, fallback : ['cglm', 'cglm_dep'])

# OpenGL muss unter Linux fast immer dynamisch bleiben (Treiber-Schnittstelle)
gl_dep = dependency('gl')
dl_dep = cc.find_library('dl', required : false)

inc = include_directories('include')

# Trenne glad.c von den restlichen Quellen
glad_res = static_library('glad', 'src/glad.c',
  include_directories : inc,
  # Hier deaktivieren wir die pedantischen Warnungen nur für diese Datei
  c_args : ['-w'] # Oder spezifischer: ['-Wno-pedantic']
)

sources = [
  'src/main.c',
  # 'src/glad.c',
  'src/shader_loader.c',
  'src/shader_utils.c',
  'src/file_utils.c',
  'src/camera.c'
]

# --- 4. Executable erstellen ---
executable('cortex',
  sources,
  # Hier binden wir die glad-Library ein
  link_with : glad_res,
  dependencies : [glfw_dep, cglm_dep, gl_dep, dl_dep],
  include_directories : inc,
  install : true,
  # -static-libgcc hilft, die C-Standard-Runtime portabler zu machen
  link_args : ['-static-libgcc']
)
