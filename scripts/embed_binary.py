#!/usr/bin/env python3
import sys
import os

def file_to_c_array(input_file, variable_name):
    if not os.path.exists(input_file):
        return None
    with open(input_file, "rb") as f:
        data = f.read()
    out = [f"const unsigned char {variable_name}[] = {{"]
    for i, byte in enumerate(data):
        if i % 12 == 0:
            out.append("\n    ")
        out.append(f"0x{byte:02x}, ")
    out.append("\n};\n")
    out.append(f"const unsigned int {variable_name}_len = {len(data)};\n")
    return "".join(out)

def main():
    if len(sys.argv) < 5:
        print("Usage: embed_binary.py <input_file> <variable_name> <output_h> <output_c>")
        sys.exit(1)
        
    input_file = sys.argv[1]
    variable_name = sys.argv[2]
    output_h = sys.argv[3]
    output_c = sys.argv[4]
    
    # Header
    with open(output_h, "w") as f:
        f.write(f"#ifndef {variable_name.upper()}_EMBEDDED_H\n")
        f.write(f"#define {variable_name.upper()}_EMBEDDED_H\n\n")
        f.write(f"extern const unsigned char {variable_name}[];\n")
        f.write(f"extern const unsigned int {variable_name}_len;\n\n")
        f.write("#endif\n")
        
    # Source
    array_code = file_to_c_array(input_file, variable_name)
    if array_code:
        with open(output_c, "w") as f:
            f.write(f'#include "{os.path.basename(output_h)}"\n\n')
            f.write(array_code)
        print(f"Generated {output_h} and {output_c}")
    else:
        sys.exit(1)

if __name__ == "__main__":
    main()
