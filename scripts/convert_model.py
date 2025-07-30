#!/usr/bin/env python

import subprocess
import tempfile
import math
import re

'''
ifn0cmple x c (c < 0) = ifcmple x c
ifn0cmple x c (c >= 0) = [swapped] ifcmpgt x c
ifnotnanandcmple x c = ifcmple x c
ifnanorcmple x c = [swapped] ifcmpgt x c
ifnotnan x = ifcmple x +INFINITY
ifnan x = [swapped] ifcmple x +INFINITY
'''


def reimprove(code):
    res = []
    for line in code.splitlines():
        line = line.strip()
        if not line:
            continue
        try:
            op, x, c, s = line.split()
        except ValueError:
            print('Error:', line)
            continue
        x = int(x)
        c = float(c)
        s = int(s)

        if op == 'ifn0cmple':
            assert s == 0
            if c < 0:
                op = 'ifcmple'
            else:
                assert c >= 0
                op = 'ifcmpgt'
                s = 1
        elif op == 'ifnotnanandcmple':
            assert s == 0
            op = 'ifcmple'
        elif op == 'ifnanorcmple':
            assert s == 0
            op = 'ifcmpgt'
            s = 1
        elif op == 'ifnotnan':
            assert s == 0
            op = 'ifcmple'
            c = 'inf'
        elif op == 'ifnan':
            assert s == 0
            op = 'ifcmple'
            c = 'inf'
            s = 1

        res.append([op, x, c, s])

    return '\n'.join(f'{op} {x} {c} {s}' for op, x, c, s in res)


def improve(code):
    code = re.sub(
        r'fval = arr\[(\d+)\];if \(std::isnan\(fval\)\) fval = 0.0;if/\*SIMA\*/\(fval <= (-?(?:inf|\d+\.\d+(?:e[+-]\d+)?))\) \{',
        r'ifn0cmple \1 \2 0\n',
        code)
    code = re.sub(
        r'fval = arr\[(\d+)\];if \(!std::isnan\(fval\)\) \{',
        r'ifnotnan \1 nan 0\n',
        code)
    code = re.sub(
        r'fval = arr\[(\d+)\];if \(std::isnan\(fval\)\) \{',
        r'ifnan \1 nan 0\n',
        code)
    code = re.sub(
        r'fval = arr\[(\d+)\];if/\*SIMA\*/\(std::isnan\(fval\) \|\|fval <= (-?(?:inf|\d+\.\d+(?:e[+-]\d+)?))\) \{',
        r'ifnanorcmple \1 \2 0\n',
        code)
    code = re.sub(
        r'fval = arr\[(\d+)\];if/\*SIMA\*/\(!std::isnan\(fval\) &&fval <= (-?(?:inf|\d+\.\d+(?:e[+-]\d+)?))\) \{',
        r'ifnotnanandcmple \1 \2 0\n',
        code)
    code = re.sub(
        r'return (-?(?:inf|\d+\.\d+(?:e[+-]\d+)?)); ',
        r'return -1 \1 -1\n',
        code)
    code = re.sub(
        r'\} else \{ ',
        r'else -1 nan -1\n',
        code)
    code = re.sub(
        r'\} ',
        r'endif -1 nan -1\n',
        code)
    return code.rstrip('\n')


def procfile(path, is_float=True, key=None, mapping=None):
    visited = {}
    with open(path, 'r') as f:
        for line in f.readlines():
            m = re.match(r'^double PredictTree(\d+)\(const double\* arr\) \{ const std::vector<uint32_t> cat_threshold = \{\};double fval = 0.0f; (.*)\}$', line)
            if not m:
                continue
            k = int(m.group(1))
            body = m.group(2)
            body = improve(body)
            body = reimprove(body)
            if mapping:
                body = revamp(body, mapping)
            visited[k] = f'defproc {k} nan -1\n{body}\nendproc -1 nan -1\n\n'
    res = ''
    for k in sorted(visited.keys()):
        res += visited[k]
    res = res.rstrip('\n')
    return finish(res, is_float, key)


def revamp(code, mapping):
    res = []
    for line in code.splitlines():
        line = line.strip()
        if not line:
            continue
        try:
            op, x, c, s = line.split()
        except ValueError:
            print('Error:', line)
            continue
        x = int(x)
        c = float(c)
        s = int(s)

        if x != -1:
            x = mapping[x]
        res.append([op, x, c, s])

    return '\n'.join(f'{op} {x} {c} {s}' for op, x, c, s in res)


def finish(code, is_float=True, key=None):
    indent = 0
    tab = '    '
    p = 'f' if is_float else ''
    ty = 'float' if is_float else 'double'

    res = f'#include <math.h>\n'

    trees = []
    for line in code.splitlines():
        line = line.strip()
        if not line:
            continue
        try:
            op, x, c, s = line.split()
        except ValueError:
            print('Error:', line)
            continue

        x = int(x)
        c = float(c)
        s = int(s)

        if math.isinf(c):
            if c < 0:
                c = '-INFINITY'
            else:
                c = 'INFINITY'
        elif math.isnan(c):
            c = f'nan{p}()'
        else:
            c = f'{c}{p}'

        if op == 'defproc':
            res += f'\nMODELDECL {ty} {key}f{x}(const {ty} *a) {{\n'
            trees.append(x)
            indent += 1

        elif op == 'ifcmple':
            res += indent * tab
            np, ne = ('(!', ')') if s == 1 else ('', '')
            res += f'if {np}(a[{x}] <= {c}){ne} {{\n'
            indent += 1

        elif op == 'ifcmpgt':
            res += indent * tab
            np, ne = ('(!', ')') if s == 1 else ('', '')
            res += f'if {np}(a[{x}] > {c}){ne} {{\n'
            indent += 1

        elif op == 'ifn0cmple':
            res += indent * tab
            np, ne = ('(!', ')') if s == 1 else ('', '')
            res += f'{ty} tmp = a[{x}]; if (isnan{p}(tmp)) {{ tmp = 0; }} if {np}(tmp <= {c}){ne} {{\n'
            indent += 1

        elif op == 'ifn0cmpgt':
            res += indent * tab
            np, ne = ('(!', ')') if s == 1 else ('', '')
            res += f'{ty} tmp = a[{x}]; if (isnan{p}(tmp)) {{ tmp = 0; }} if {np}(tmp > {c}){ne} {{\n'
            indent += 1

        elif op == 'ifnan':
            res += indent * tab
            np, ne = ('(!', ')') if s == 1 else ('', '')
            res += f'if {np}(isnan{p}(a[{x}])){ne} {{\n'
            indent += 1

        elif op == 'ifnotnan':
            res += indent * tab
            np, ne = ('(!', ')') if s == 1 else ('', '')
            res += f'if {np}(!isnan{p}(a[{x}])){ne} {{\n'
            indent += 1

        elif op == 'ifnanorcmple':
            res += indent * tab
            np, ne = ('(!', ')') if s == 1 else ('', '')
            res += f'if {np}(isnan{p}(a[{x}]) || a[{x}] <= {c}){ne} {{\n'
            indent += 1

        elif op == 'ifnotnanandcmple':
            res += indent * tab
            np, ne = ('(!', ')') if s == 1 else ('', '')
            res += f'if {np}(!isnan{p}(a[{x}]) && a[{x}] <= {c}){ne} {{\n'
            indent += 1

        elif op == 'endif':
            indent -= 1
            res += indent * tab
            res += f'}}\n'

        elif op == 'endproc':
            indent -= 1
            res += indent * tab
            res += f'}}\n'

        elif op == 'else':
            indent -= 1
            res += indent * tab
            res += f'}} else {{\n'
            indent += 1

        elif op == 'return':
            res += indent * tab
            res += f'return {c};\n'

        else:
            raise ValueError(op)

    res += f'''
MODELDECL {ty} {key}f(const {ty} *a) {{
{tab}return ({' + '.join(f'{key}f{i}(a)' for i in reversed(trees))}); // * (1.0{p} / {len(trees)}.0{p});
}}
'''
    return res


model_version = '20250604'
benchmark_values = {
    'regression': 0.003,
    'classification': 0.55,
}


mapping = {}

print('Loading feature names')
feature_names = []
with open(f'/data/daily_csv/lgbm_column_fking_order.txt', 'r') as f:
    for line in f.readlines():
        line = line.strip()
        if not line:
            continue
        line = line.replace('.', 'o')
        feature_names.append(line)
print(f'Loaded {len(feature_names)} features')

print('Loading feature order')
feature_order = []
with open(f'src/FactorEnum.h', 'r') as f:
    for line in f.readlines():
        line = line.strip()
        if line.endswith(','):
            feature_order.append(line.removesuffix(',').strip())

print('Computing feature mapping')
for i, name in enumerate(feature_names):
    mapping[i] = feature_order.index(name)


is_float = False

print('Converting models')
models = {}
for k in ['regression', 'classification']:
    # if not os.path.exists(f'{k}_model.cpp'):
    with tempfile.NamedTemporaryFile('w') as f:
        f.write(f'''\
task = convert_model
boosting_type = gbdt
convert_model_language = cpp
input_model = /data/daily_csv/{k}_model_{model_version}.txt
convert_model = /data/daily_csv/{k}_model_{model_version}_reorder.cpp
''')
        f.flush()
        subprocess.check_call(['lightgbm', f'config={f.name}'])

    models[k] = procfile(f'/data/daily_csv/{k}_model_{model_version}_reorder.cpp', is_float=is_float, key=k, mapping=mapping)

if is_float:
    ty = 'float'
    p = 'f'
else:
    ty = 'double'
    p = ''

print('Generating C++ codes')
for k, code in models.items():
    print(k)
    with open(f'src/_generated_model_{k}.inl', 'w') as f:
        f.write(code)
        f.write(f'''
MODELDECL {ty} {k}(const {ty} *a) {{
    ''')
        if k == 'classification':
            f.write(f'return 1.0{p} / (1.0{p} + exp{p}(-{k}f(a))) - {benchmark_values[k]}{p};')
        else:
            f.write(f'return {k}f(a) - {benchmark_values[k]}{p};')
        f.write(f'''
}}
''')
