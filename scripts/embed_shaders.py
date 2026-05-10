#!/usr/bin/env python3
import os
import sys

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    
    shader_dir = os.path.join(project_root, 'shaders')
    output_file = 'shaders_embedded.h'
    
    # Mapping table to ensure variable names in C match exactly what main.c expects
    shaders = {
        'vertex.glsl': 'vertex_shader_source',
        'fragment.glsl': 'fragment_shader_source',
        'quad_vertex.glsl': 'quad_vertex_shader_source',
        'quad_fragment.glsl': 'quad_fragment_shader_source',
        'ui_vertex.glsl': 'ui_vertex_shader_source',
        'ui_fragment.glsl': 'ui_fragment_shader_source'
    }

    with open(output_file, 'w') as f:
        f.write('#ifndef SHADERS_EMBEDDED_H\n')
        f.write('#define SHADERS_EMBEDDED_H\n\n')
        
        for filename, var_name in shaders.items():
            path = os.path.join(shader_dir, filename)
            
            if not os.path.exists(path):
                print(f"Warning: {path} not found. Skipping.")
                continue
                
            with open(path, 'r') as s_file:
                content = s_file.read()
                
            f.write(f'static const char* {var_name} = \n')
            for line in content.splitlines():
                escaped = line.replace('\\', '\\\\').replace('"', '\\"')
                f.write(f'"{escaped}\\n"\n')
            f.write(';\n\n')
            
        f.write('#endif\n')

if __name__ == "__main__":
    main()
