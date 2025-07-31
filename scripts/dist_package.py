#!/usr/bin/env python



targets = {
    'NESH': [
        '-DCMAKE_BUILD_TYPE=Release',
        '-DBUILD_SPEED=ON',
        '-DREPLAY=OFF',
        '-DASYNC_LOGGER=OFF',
        '-DRECORD=ON',
        '-DBUILD_FOR_NE=ON',
        '-DBUILD_FOR_SZ=OFF',
    ],
    # 'NESZ': [
    #     '-DCMAKE_BUILD_TYPE=Release',
    #     '-DBUILD_SPEED=ON',
    #     '-DREPLAY=OFF',
    #     '-DASYNC_LOGGER=OFF',
    #     '-DRECORD=ON',
    #     '-DBUILD_FOR_NE=ON',
    #     '-DBUILD_FOR_SZ=ON',
    # ],
}

markets = [
    'sh',
    'sz',
]

securities = [
    'ne',
    # 'xc',
]


import os
import json
import shutil
import subprocess
import datetime
import sys

today = sys.argv[1] if len(sys.argv) > 1 else datetime.datetime.now().strftime('%Y%m%d')

os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
for market in markets:
    subprocess.check_call(['/opt/miniconda3/bin/python', 'scripts/fetch_state.py', market.upper(), today])

shutil.rmtree('dist', ignore_errors=True)
os.makedirs('dist/bin', exist_ok=True)
os.makedirs('dist/log', exist_ok=True)
os.makedirs('dist/config', exist_ok=True)

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


for target, options in targets.items():
    shutil.rmtree(f'build/{target}', ignore_errors=True)
    subprocess.check_call([
        'cmake', '-B', f'build/{target}', '-G', 'Ninja', '-Wno-dev',
    ] + options)
    subprocess.check_call([
        'cmake', '--build', f'build/{target}', '--target', 'mdd_v2',
    ])

for market in markets:
    shutil.copyfile(f'/data/daily_csv/mdd2_factors_{market}_{today}.bin', f'dist/config/factors_{market}.bin')

for target in targets:
    shutil.copyfile(f'build/{target}/mdd_v2', f'dist/bin/mdd_v2.{target}')
    with open(f'config/{target}.json', 'rb') as f:
        config = json.load(f)
    config['date'] = int(today)
    market = target[-2:].lower()
    config['factor_file'] = f'../config/factors_{market}.bin'
    with open(f'dist/config/{target}.json', 'w') as f:
        json.dump(config, f)

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
exec -a mdd_v2.{target} ../bin/ld-linux-x86-64.so.2 ../bin/mdd_v2.{target} ../config/{target}.json
''')
    subprocess.check_call(['chmod', '+x', f'dist/bin/start.{target}'])

subprocess.check_call(['tar', 'zcvf', f'/data/release/MDD-{today}.tar.gz', '.'], cwd='dist')
for security in securities:
    for market in markets:
        subprocess.check_call([f'scripts/{security}-upload.sh', market, f'/data/release/MDD-{today}.tar.gz', f'/root/MDD-{today}.tar.gz'])
        with subprocess.Popen([f'scripts/{security}-connect.sh', market], stdin=subprocess.PIPE) as p:
            p.communicate(f'''set +o history
set -e
cd /root
rm -rf {today}
mkdir -p {today}
cd {today}
tar zxvf ../MDD-{today}.tar.gz
cd ..
rm -f MDD-{today}.tar.gz
rm -f today
ln -sf {today} today
echo /root/today/bin/start.{security.upper()}{market.upper()} > start.sh
chmod +x start.sh
'''.encode())
