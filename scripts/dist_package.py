#!/usr/bin/env python


common_options = [
    '-DCMAKE_BUILD_TYPE=Release',
    '-DBUILD_SPEED=ON',
    '-DRECORD_FACTORS=ON',
    '-DASYNC_LOGGER=OFF',
    '-DNO_EXCEPTION=ON',
    '-DDETAIL_LOG=ON',
    '-DSELL_GC001=OFF',
    '-DBYPASS_OES=OFF',
]

targets = {
    'XCSH': [
        '-DALWAYS_BUY=OFF',
        '-DDUMMY_QUANTITY=ON',
    ],

    'NESZ': [
        '-DALWAYS_BUY=OFF',
        '-DDUMMY_QUANTITY=ON',
    ],

    'NESH': [
        '-DALWAYS_BUY=OFF',
        '-DDUMMY_QUANTITY=ON',
    ],

    'NESZ': [
        '-DALWAYS_BUY=OFF',
        '-DDUMMY_QUANTITY=ON',
    ],

    'NESZ2': [
        '-DALWAYS_BUY=OFF',
        '-DDUMMY_QUANTITY=ON',
    ],
}


import os
import json
import shutil
import subprocess
import datetime
import sys

today = datetime.datetime.now().strftime('%Y%m%d')
if len(sys.argv) > 1 and any(k in sys.argv[1:] for k in targets):
    targets = {k: v for k, v in targets.items() if k in sys.argv[1:]}
    try:
        int(sys.argv[1])
    except ValueError:
        pass
    else:
        today = sys.argv[1]

print('today:', today)
print('targets:', targets)

# print(f'-- Fetching market state')
# os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
# for market in ['sh', 'sz']:
#     subprocess.check_call(['/opt/miniconda3/bin/python', 'scripts/fetch_state.py', market.upper(), today])

def patch_file(path):
    subprocess.check_call(['chmod', '+x', path])
    subprocess.check_call(['patchelf', '--set-rpath', '$ORIGIN', path])

blacklist = ['linux-vdso.so.1', 'ld-linux-x86-64.so.2']

def prepare_file(out, *paths):
    paths = set(paths)
    result = set(paths)

    stack = list(paths)
    while stack:
        path = stack.pop()
        for line in subprocess.check_output(['ldd', os.path.join(out, path)]).decode().splitlines():
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
        dest = os.path.join(out, os.path.basename(path))
        print('Copying', path)
        shutil.copyfile(path, dest)
        patch_file(dest)

    for path in files:
        dest = os.path.join(out, path)
        print('Patching', path)
        patch_file(dest)


for target, options in targets.items():
    shutil.rmtree(f'build/{target}', ignore_errors=True)
    market = target[2:4].lower()
    is_second = target.endswith('2')
    security = target[:2].lower()
    target_options = [
        f'-DTARGET_SECURITY={security}',
        f'-DTARGET_MARKET={market}',
        f'-DSZ_IS_SECOND={"ON" if is_second else "OFF"}',
    ]
    options = common_options + target_options + options
    print(f'-- Building {target}: {options}')
    subprocess.check_call([
        'cmake', '-B', f'build/{target}', '-G', 'Ninja', '-Wno-dev',
    ] + options)
    subprocess.check_call([
        'cmake', '--build', f'build/{target}', '--target', 'mdd_v2',
    ])

shutil.rmtree(f'dist', ignore_errors=True)
for target in targets:
    os.makedirs(f'dist/{target}', exist_ok=True)
    print(f'-- Packing {target}')

    market = target[2:4].lower()
    security = target[:2].lower()

    shutil.copyfile(f'build/{target}/mdd_v2', f'dist/{target}/mdd_v2')
    shutil.copyfile('/lib64/ld-linux-x86-64.so.2', f'dist/{target}/ld-linux-x86-64.so.2')
    shutil.copyfile('/lib/x86_64-linux-gnu/librt.so.1', f'dist/{target}/librt.so.1')
    shutil.copyfile('/lib/x86_64-linux-gnu/libdl.so.2', f'dist/{target}/libdl.so.2')
    subprocess.check_call(['chmod', '+x', f'dist/{target}/ld-linux-x86-64.so.2'])

    files = ['librt.so.1', 'libdl.so.2', 'mdd_v2']
    prepare_file(f'dist/{target}', *files)
    subprocess.check_call(['patchelf', '--set-interpreter', '/root/MDD/ld-linux-x86-64.so.2', f'dist/{target}/mdd_v2'])

    shutil.copyfile(f'/data/daily_csv/mdd2_factors_{market}_{today}.bin', f'dist/{target}/factors_{market}_{today}.bin')
    with open(f'config/{target}.json', 'rb') as f:
        config = json.load(f)
    config['date'] = int(today)
    market = target[-2:].lower()
    config['factor_file'] = f'/root/MDD/factors_{market}_{today}.bin'
    with open(f'dist/{target}/config_{target}_{today}.json', 'w') as f:
        json.dump(config, f)

    with open(f'dist/{target}/start', 'w') as f:
        f.write(rf'''#!/bin/bash
set -e
cd `dirname "$0"`
wd=$PWD
mkdir -p ../log
cd ../log

exec -a mdd_v2 $wd/ld-linux-x86-64.so.2 $wd/mdd_v2 $wd/config_{target}_{today}.json'
''')
    subprocess.check_call(['chmod', '+x', f'dist/{target}/start'])

for target in targets:
    print(f'-- Compressing {target}')
    subprocess.check_call(['tar', 'zcvf', f'/data/release/MDD-{target}-{today}.tar.gz', '.'], cwd=f'dist/{target}')

for target in targets:
    print(f'-- Publishing {target}')
    market_full = target[2:].lower()
    security = target[:2].lower()
    subprocess.check_call([f'scripts/{security}-upload.sh', market_full, f'/data/release/MDD-{target}-{today}.tar.gz', f'/tmp/MDD-{target}-{today}.tar.gz'])
    with subprocess.Popen([f'scripts/{security}-connect.sh', market_full], stdin=subprocess.PIPE) as p:
        p.communicate(f'''set +o history
set -e
cd /root
rm -rf /root/MDD
mkdir -p /root/MDD
cd /root/MDD
tar zxvf /tmp/MDD-{target}-{today}.tar.gz
cd /root
rm -f /tmp/MDD-{target}-{today}.tar.gz
mkdir -p /root/log
echo exec /root/MDD/start > /root/start.sh
chmod +x /root/start.sh
'''.encode())
