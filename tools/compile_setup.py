import re
import json
import sys
import os

def parse_setup(filepath):
    with open(filepath, 'r') as f:
        content = f.read()

    # Remove C-style comments
    content = re.sub(r'//.*', '', content)
    
    lines = [line.strip() for line in content.split('\n') if line.strip()]

    setup_data = {
        'constants': {},
        'macros': []
    }

    current_block = []

    for line in lines:
        # Parse #define
        define_match = re.match(r'^#define\s+(\w+)\s+(.+)$', line)
        if define_match:
            name = define_match.group(1)
            val = define_match.group(2).strip()
            # If it's a hex value or integer
            setup_data['constants'][name] = val
            continue

        # Parse macro calls: MacroName(arg1, arg2, arg3)
        macro_match = re.match(r'^(\w+)\((.*)\)$', line)
        if macro_match:
            name = macro_match.group(1)
            args_str = macro_match.group(2)
            args = [arg.strip() for arg in args_str.split(',')] if args_str else []
            setup_data['macros'].append({
                'name': name,
                'args': args
            })
            continue

        # Parse standalone macros (e.g. End_If())
        if line.endswith('()'):
            setup_data['macros'].append({
                'name': line[:-2],
                'args': []
            })
            continue

        # Other unhandled lines
        print(f"Warning: could not parse line: {line}")

    return setup_data

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python3 compile_setup.py <input_file>")
        sys.exit(1)
    
    result = parse_setup(sys.argv[1])
    
    out_file = os.path.splitext(sys.argv[1])[0] + '.json'
    with open(out_file, 'w') as f:
        json.dump(result, f, indent=4)
    
    print(f"Compiled to {out_file}")
