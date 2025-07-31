#!/bin/bash

set -e
cd "$(dirname "$0")"/..

market=$1
if [ "x$market" != "xsh" ] && [ "x$market" != "xsz" ]; then
    echo "Usage:" >&2
    echo "  $0 sh" >&2
    echo "  $0 sz" >&2
    exit 1
fi

local=$2
if [ "x$local" == "x" ]; then
    echo "Usage:" >&2
    echo "  $0 $market local-file.txt /some/remote/file.txt" >&2
    exit 1
fi
remote=$3
if [ "x$remote" == "x" ]; then
    echo "Usage:" >&2
    echo "  $0 $market local-file.txt /some/remote/file.txt" >&2
    exit 1
fi

scripts/get_file.sh $market $local $remote
