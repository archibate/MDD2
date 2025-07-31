#!/bin/bash

set -e

market=$1
if [ "x$market" == "xsh" ]; then
    port=9622
elif [ "x$market" == "xsz" ]; then
    port=9822
else
    echo "Usage:" >&2
    echo "  $0 sh /some/remote/file.txt file.txt" >&2
    echo "  $0 sz /some/remote/file.txt file.txt" >&2
    exit 1
fi

remote=$2
if [ "x$remote" == "x" ]; then
    echo "Usage:" >&2
    echo "  $0 $market /some/remote/file.txt local-file.txt" >&2
    exit 1
fi
local=$3
if [ "x$local" == "x" ]; then
    echo "Usage:" >&2
    echo "  $0 $market /some/remote/file.txt local-file.txt" >&2
    exit 1
fi

sshpass -p 1 scp -P $port root@10.2.0.16:$remote $local
