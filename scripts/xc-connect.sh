#!/bin/bash

set -e

market=$1
if [ "x$market" != "xsh" ] && [ "x$market" != "xsz" ]; then
    echo "Usage:" >&2
    echo "  $0 sh" >&2
    echo "  $0 sz" >&2
    exit 1
fi

if tty > /dev/null 2>&1; then
    stty -echo raw
fi

if [ "x$market" == "xsh" ]; then
    (sleep 6; echo abcABC123@xzd; cat; echo; echo exit) | sshpass -p Xcsc@20250403 ssh -p 222 10.208.40.214 -l ljz_srsm_sh01@root@10.208.40.217
elif [ "x$market" == "xsz" ]; then
    lock=$(mktemp)
    (sleep 6; echo abcABC123@xzd; sleep 1; echo; echo exec sshpass -p abcABC123@xzd ssh 10.33.57.86; sleep 13; echo; cat; echo; echo exit; while test -f $lock; do sleep 0.2; done) | (sshpass -p Xcsc@20250403 ssh -p 222 10.208.40.214 -l ljz_srsm_sh01@root@10.208.40.217; rm -f ${lock?lock})
fi

if tty > /dev/null 2>&1; then
    stty sane
fi
