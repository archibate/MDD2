#!/usr/bin/env python



targets = {
    'NESH': [
        '-DCMAKE_BUILD_TYPE=Release',
        '-DBUILD_SPEED=ON',
        '-DRECORD_FACTORS=ON',
        '-DALWAYS_BUY=OFF',
        '-DASYNC_LOGGER=OFF',
        '-DTARGET_SECURITY=NE',
        '-DTARGET_MARKET=SH',
        '-DASYNC_LOGGER=OFF',
    ],
    'NESZ': [
        '-DCMAKE_BUILD_TYPE=Release',
        '-DBUILD_SPEED=ON',
        '-DRECORD_FACTORS=ON',
        '-DALWAYS_BUY=OFF',
        '-DASYNC_LOGGER=OFF',
        '-DTARGET_SECURITY=NE',
        '-DTARGET_MARKET=SH',
    ],
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
        dest = os.path.join('bin', os.path.basename(path))
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

shutil.rmtree(f'dist', ignore_errors=True)
for target in targets:
    os.makedirs(f'dist/{target}', exist_ok=True)
    os.chdir(f'dist/{target}')

    os.makedirs('bin', exist_ok=True)
    os.makedirs('log', exist_ok=True)
    os.makedirs('config', exist_ok=True)

    market = target[-2:].lower()
    security = target[:2].lower()

    shutil.copyfile(f'/data/daily_csv/mdd2_factors_{market}_{today}.bin', f'config/factors.bin')
    with open(f'../../config/{target}.json', 'rb') as f:
        config = json.load(f)
    config['date'] = int(today)
    market = target[-2:].lower()
    config['factor_file'] = f'../config/factors.bin'
    with open('config/config.json', 'w') as f:
        json.dump(config, f)

    shutil.copyfile(f'../../build/{target}/mdd_v2', 'bin/mdd_v2')
    shutil.copyfile('/lib64/ld-linux-x86-64.so.2', 'bin/ld-linux-x86-64.so.2')
    shutil.copyfile('/lib/x86_64-linux-gnu/librt.so.1', 'bin/librt.so.1')
    shutil.copyfile('/lib/x86_64-linux-gnu/libdl.so.2', 'bin/libdl.so.2')
    subprocess.check_call(['chmod', '+x', 'bin/ld-linux-x86-64.so.2'])

    files = ['bin/librt.so.1', 'bin/libdl.so.2', f'bin/mdd_v2']
    prepare_file(*files)
    strip_file(*files)

    with open('bin/start', 'w') as f:
        f.write(rf'''#!/bin/bash
set -e
mkdir -p `dirname "$0"`/../log
cd `dirname "$0"`/../log

export XELE_MD_DISABLE_AUTH=1
exec -a mdd_v2 ../bin/ld-linux-x86-64.so.2 ../bin/mdd_v2 ../config/config.json
''')
    subprocess.check_call(['chmod', '+x', 'bin/start'])
    os.chdir('../..')

for target in targets:
    subprocess.check_call(['tar', 'zcvf', f'/data/release/MDD-{target}-{today}.tar.gz', '.'], cwd=f'dist/{target}')

for target in targets:
    market = target[-2:].lower()
    security = target[:2].lower()
    subprocess.check_call([f'scripts/{security}-upload.sh', market, f'/data/release/MDD-{target}-{today}.tar.gz', f'/root/MDD-{market}-{today}.tar.gz'])
    with subprocess.Popen([f'scripts/{security}-connect.sh', market], stdin=subprocess.PIPE) as p:
        p.communicate(f'''set +o history
set -e
cd /root
rm -rf {today}
mkdir -p {today}
cd {today}
tar zxvf ../MDD-{market}-{today}.tar.gz
cd ..
rm -f MDD-{market}-{today}.tar.gz
rm -f today
ln -sf {today} today
echo /root/today/bin/start > start.sh
chmod +x start.sh
'''.encode())
