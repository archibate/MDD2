#!/bin/bash

set -e

market=$1
if [ "x$market" == "xsh" ]; then
    port=9622
elif [ "x$market" == "xsz" ]; then
    port=9822
else
    echo "Usage:" >&2
    echo "  $0 sh" >&2
    echo "  $0 sz" >&2
    exit 1
fi

exec sshpass -p 1 ssh root@10.2.0.16 -p $port
