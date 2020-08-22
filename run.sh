#!/bin/zsh
PCAPDIR=/mnt/pcaps
BIN=./buildroot/bin/pcap_analyzer

if [[ ! -e $BIN ]]; then
   make -j8
fi

if [[ -e db.pid ]]; then
    kill -9 $(cat db.pid)
fi

python3 scripts/db_update_service.py 2>&1 > output.txt &
PID=$!
echo "$PID" > db.pid

$BIN -d $PCAPDIR -o br1_output.txt -z tcp://127.0.0.1:5555 -v --timeout 5000
sleep 1
kill -9 $PID
if [[ -e db.pid ]]; then
    rm db.pid
fi

