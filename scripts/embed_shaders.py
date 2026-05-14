#!/usr/bin/env python3
import os

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    shader_dir = os.path.join(project_root, 'shaders')
    output_h = 'shaders_embedded.h'
    output_c = 'shaders_embedded.c'
    
    found_shaders = []
    
    # Rekursiv nach .vert und .frag suchen
    for root, dirs, files in os.walk(shader_dir):
        for file in files:
            if file.endswith('.vert') or file.endswith('.frag') or file.endswith('.glsl'):
                rel_path = os.path.relpath(os.path.join(root, file), shader_dir)
                # Variablen-Name aus Pfad generieren: core/default.vert -> core_default_vert
                var_name = rel_path.replace(os.sep, '_').replace('.', '_') + '_source'
                found_shaders.append((os.path.join(root, file), var_name))

    # Header schreiben
    with open(output_h, 'w') as f:
        f.write('#ifndef SHADERS_EMBEDDED_H\n')
        f.write('#define SHADERS_EMBEDDED_H\n\n')
        for _, var_name in found_shaders:
            f.write(f'extern const char* {var_name};\n')
        f.write('\n#endif\n')

    # Source schreiben
    with open(output_c, 'w') as f:
        f.write(f'#include "{output_h}"\n\n')
        for full_path, var_name in found_shaders:
            with open(full_path, 'r') as s_file:
                content = s_file.read()
            f.write(f'const char* {var_name} = \n')
            for line in content.splitlines():
                escaped = line.replace('\\', '\\\\').replace('"', '\\"')
                f.write(f'"{escaped}\\n"\n')
            f.write(';\n\n')

if __name__ == "__main__":
    main()
