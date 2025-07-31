#!/usr/bin/env python

import subprocess
import time
import os



def flamegraph():
    os.chdir('build')
    try:
        subprocess.run('perf record -e cycles -g --call-graph fp ./mdd_v2', shell=True)
    except KeyboardInterrupt:
        time.sleep(1)
    subprocess.check_call(r'''perf script | sed 's/ (/$SLB$/g' | sed 's/(/$LB$/g' | sed 's/)/$RB$/g' | sed 's/\$SLB\$/ (/g' | sed 's/\$RB\$$/)/g' | /opt/FlameGraph/stackcollapse-perf.pl | /opt/FlameGraph/flamegraph.pl | sed 's/\$LB\$/(/g' | sed 's/\$RB\$/)/g' > flamegraph.svg''', shell=True)
    subprocess.check_call('perf report -g graph', shell=True)
    # subprocess.check_call('display flamegraph.svg', shell=True)


def callgrind():
    os.chdir('build')
    try:
        subprocess.run('valgrind --tool=callgrind --callgrind-out-file=callgrind.out ./mdd2', shell=True)
    except KeyboardInterrupt:
        time.sleep(1)
    subprocess.check_call('kcachegrind callgrind.out', shell=True)


def build():
    os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
    subprocess.check_call(['cmake', '-B', 'build'])
    subprocess.check_call(['cmake', '--build', 'build', '--target', 'mdd_v2'])


build()
flamegraph()
