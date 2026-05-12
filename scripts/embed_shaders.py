#!/usr/bin/env python3
import os
import sys

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    
    shader_dir = os.path.join(project_root, 'shaders')
    output_h = 'shaders_embedded.h'
    output_c = 'shaders_embedded.c'
    
    shaders = {
        'vertex.glsl': 'vertex_shader_source',
        'fragment.glsl': 'fragment_shader_source',
        'quad_vertex.glsl': 'quad_vertex_shader_source',
        'quad_fragment.glsl': 'quad_fragment_shader_source',
        'ui_vertex.glsl': 'ui_vertex_shader_source',
        'ui_fragment.glsl': 'ui_fragment_shader_source',
        'text_vertex.glsl': 'text_vertex_shader_source',
        'text_fragment.glsl': 'text_fragment_shader_source'
    }

    # Write Header
    with open(output_h, 'w') as f:
        f.write('#ifndef SHADERS_EMBEDDED_H\n')
        f.write('#define SHADERS_EMBEDDED_H\n\n')
        for var_name in shaders.values():
            f.write(f'extern const char* {var_name};\n')
        f.write('\n#endif\n')

    # Write Source
    with open(output_c, 'w') as f:
        f.write(f'#include "{output_h}"\n\n')
        for filename, var_name in shaders.items():
            path = os.path.join(shader_dir, filename)
            if not os.path.exists(path):
                continue
            with open(path, 'r') as s_file:
                content = s_file.read()
            f.write(f'const char* {var_name} = \n')
            for line in content.splitlines():
                escaped = line.replace('\\', '\\\\').replace('"', '\\"')
                f.write(f'"{escaped}\\n"\n')
            f.write(';\n\n')

if __name__ == "__main__":
    main()
