#!/bin/zsh
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

$BIN -d ./pcaps -o br1_output.txt -z tcp://127.0.0.1:5555 -v
sleep 1
kill -9 $PID
if [[ -e db.pid ]]; then
    rm db.pid
fi

