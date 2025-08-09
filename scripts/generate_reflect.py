#!/usr/bin/env python

import re
import os
import sys


registry = {}
class_name = ''

for file_path in sys.argv[1:]:
    with open(file_path, 'r', encoding='latin1') as f:

        for line in f.readlines():
            line = line.strip()

            m = re.match(r'^struct\s+([A-Za-z0-9_]+)(?:\s*{)?$', line)
            if m:
                class_name = m.group(1)
                # print(class_name)
                registry[class_name] = {}

            m = re.match(r'^([A-Za-z0-9_]+)\s+([A-Za-z0-9_]+)(?:\s*=\s*[^;]+)?\s*;\s*(?://.*|/\*.*\*/)?$', line)
            if m and class_name:
                member_type = m.group(1)
                member_name = m.group(2)
                # print(member_type, member_name)
                registry[class_name][member_name] = member_type


for class_name in registry:
    print(f'REFLECT_BEGIN({class_name})')
    for member_name in registry[class_name]:
        print(f'REFLECT_MEMBER({member_name})')
    print(f'REFLECT_END()')
