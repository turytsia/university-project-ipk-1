#!/bin/bash
# @author xturyt00

MODE=
PORT=
HOST=


TMP="./test/temp.log"

TCP_IN=./test/tcp/in.log
TCP_OUT=./test/tcp/out.log

UDP_IN=./test/udp/in.log
UDP_OUT=./test/udp/out.log

eval make
eval touch $TMP

if [ "$MODE" == "tcp" ];
then
cat $TCP_IN | ./ipkcpc -h $HOST -p $PORT -m $MODE > $TMP
eval diff $TMP $TCP_OUT
elif [ "$MODE" == "udp" ];
then
cat $UDP_IN | ./ipkcpc -h $HOST -p $PORT -m $MODE > $TMP &
PID=$!
kill -SIGINT $PID
eval diff $TMP $UDP_OUT
else
echo "Unknown mode"
exit 1
fi

eval rm $TMP
eval make clean