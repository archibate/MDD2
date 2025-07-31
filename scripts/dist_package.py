#!/usr/bin/env python3

import os
import shutil
import subprocess


shutil.rmtree('dist', ignore_errors=True)
os.makedirs('dist/bin', exist_ok=True)
os.makedirs('dist/log', exist_ok=True)

def strip_file(*paths):
    for path in paths:
        subprocess.check_call(['chmod', '+x', path])
        subprocess.check_call(['strip', path])
        subprocess.check_call(['patchelf', '--set-rpath', '$ORIGIN', path])

blacklist = ['linux-vdso.so.1', 'ld-linux-x86-64.so.2']

def prepare_file(*paths):
    paths = set(paths)
    result = set(paths)

    stack = list(paths)
    while stack:
        path = stack.pop()
        for line in subprocess.check_output(['ldd', path]).decode().splitlines():
            line = line.strip()
            if '=>' not in line:
                continue
            l, m = line.split('=>')
            if l in blacklist:
                continue
            if '(' not in m:
                continue
            r, _ = m.split('(')
            l = l.strip()
            r = r.strip()
            if r in result:
                continue
            result.add(r)
            stack.append(r)

    for path in result:
        if path in paths:
            continue
        dest = os.path.join('dist/bin', os.path.basename(path))
        shutil.copyfile(path, dest)
        print('Copying', path)
        strip_file(dest)


targets = ['NESH', 'NESZ']

for target in targets:
    shutil.copyfile(f'build/{target}/mdd_v2', f'dist/bin/mdd_v2.{target}')

shutil.copyfile('/lib64/ld-linux-x86-64.so.2', 'dist/bin/ld-linux-x86-64.so.2')
shutil.copyfile('/lib/x86_64-linux-gnu/librt.so.1', 'dist/bin/librt.so.1')
shutil.copyfile('/lib/x86_64-linux-gnu/libdl.so.2', 'dist/bin/libdl.so.2')
subprocess.check_call(['chmod', '+x', 'dist/bin/ld-linux-x86-64.so.2'])

files = ['dist/bin/librt.so.1', 'dist/bin/libdl.so.2'] + [f'dist/bin/mdd_v2.{target}' for target in targets]
prepare_file(*files)
strip_file(*files)

for target in targets:
    with open(f'dist/bin/start.{target}', 'w') as f:
        f.write(rf'''#!/bin/bash
set -e
mkdir -p `dirname "$0"`/../log
cd `dirname "$0"`/../log

export XELE_MD_DISABLE_AUTH=1
exec ../bin/ld-linux-x86-64.so.2 ../bin/mdd_v2.{target} ../configs/{target}.json
''')
    subprocess.check_call(['chmod', '+x', f'dist/bin/start.{target}'])

subprocess.check_call(['tar', 'zcvf', f'dist.tar.gz', '.'], cwd='dist')
