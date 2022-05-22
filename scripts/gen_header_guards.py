#!/bin/env python3

import argparse
import os
from pathlib import Path

from attr import get_run_validators


def header_files(path):
    for root, dirs, files in os.walk(path):
        for file in files:
            f = Path(root) / file
            if f.suffix in ['.h', '.hpp']:
                yield f


def find_guard(lines):
    state = 'ifndef'
    start = 0
    indent = 0
    for i, line in enumerate(lines):
        if state == 'ifndef':
            if line.startswith("#"):
                if not line.startswith("#ifndef"):
                    return None
                state = 'define'
        elif state == 'define':
            if not line.startswith('#define'):
                return None
            start = i - 1
            state = 'endif'
        elif state == 'endif':
            if line.startswith('#endif') and indent == 0:
                return start, i
            if line.strip().startswith('#if'):
                indent += 1
            if line.strip().startswith('#endif'):
                indent -= 1
    return None


   

def has_license(lines):
    return len(lines) >= 3 and lines[:3] == [
        '/*\n',
        ' * SPDX-License-Identifier: Apache-2.0\n', 
        ' */\n'
    ]


def main():
    parser = argparse.ArgumentParser(description='Apply header guards to .h/.hpp files')
    parser.add_argument('path', help='Path to folder work in')
    parser.add_argument('--prefix', help='Prefix', default='')
    args = parser.parse_args()

    lines = []
    for f in header_files(args.path):
        print(f)
        with open(f, 'r') as fin:
            lines = fin.readlines()

        guard_name = args.prefix + ('_' if args.prefix else '') + str(f.relative_to(args.path)).replace('.', '_').replace('/', '_').upper() + '_'
        guard = find_guard(lines)
        license = has_license(lines)
        if guard is not None:
            start, end = guard
            if lines[start].split()[1] != guard_name:
                lines[start] = f'#ifndef {guard_name}\n'
                lines[start+1] = f'#define {guard_name}\n'
                lines[end] = f'#endif  /* {guard_name} */\n'
        else:
            lines = (lines[:3] if license else []) + [
                f'#ifndef {guard_name}\n',
                f'#define {guard_name}\n',
            ] + (lines[3:] if license else lines) + [
                ''
                f'#endif  /* {guard_name} */\n',
            ]
        if not license:
            lines = [
                '/*\n',
                ' * SPDX-License-Identifier: Apache-2.0\n', 
                ' */\n'
            ] + lines
        with open(f, 'w') as fout:
            fout.write(''.join(lines))



# Es gibt einen header guard
#     erster #ifndef
#     danach kommt define das gleiche
#     letzte zeile ohne whitespace: endif

# Es gibt keinen header guard
#     erster # ist nicht #ifndef







if __name__ == '__main__':
    main()