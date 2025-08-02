#!/usr/bin/env python

import shutil
import subprocess
import time
import os


os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))


def flamegraph():
    os.chdir('build/perf-build')
    try:
        subprocess.run('perf record -e cycles -g --call-graph fp ./mdd_v2', shell=True)
    except KeyboardInterrupt:
        time.sleep(1)
    subprocess.check_call(r'''perf script | sed 's/ (/$SLB$/g' | sed 's/(/$LB$/g' | sed 's/)/$RB$/g' | sed 's/\$SLB\$/ (/g' | sed 's/\$RB\$$/)/g' | /opt/FlameGraph/stackcollapse-perf.pl | /opt/FlameGraph/flamegraph.pl | sed 's/\$LB\$/(/g' | sed 's/\$RB\$/)/g' > flamegraph.svg''', shell=True)
    subprocess.check_call('perf report -g graph', shell=True)
    # subprocess.check_call('display flamegraph.svg', shell=True)


def callgrind():
    os.chdir('build/perf-build')
    try:
        subprocess.run('valgrind --tool=callgrind --callgrind-out-file=callgrind.out ./mdd_v2', shell=True)
    except KeyboardInterrupt:
        time.sleep(1)
    subprocess.check_call('kcachegrind callgrind.out', shell=True)


options = [
    '-DCMAKE_BUILD_TYPE=RelWithDebInfo',
    '-DBUILD_SPEED=ON',
    '-DRECORD_FACTORS=OFF',
    '-DASYNC_LOGGER=OFF',
    '-DALWAYS_BUY=OFF',
    '-DTARGET_SECURITY=REPLAY',
    '-DTARGET_MARKET=SH',
]


def build():
    shutil.rmtree('build/perf-build', ignore_errors=True)
    os.makedirs('build/perf-build', exist_ok=True)
    shutil.copyfile('config/REPLAY.json', 'build/perf-build/config.json')
    subprocess.check_call(['cmake', '-B', 'build/perf-build'] + options)
    subprocess.check_call(['cmake', '--build', 'build/perf-build', '--target', 'mdd_v2'])


build()
flamegraph()
# callgrind()
