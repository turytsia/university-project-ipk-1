#!/bin/bash
# @author xturyt00
HOST=192.168.43.20
PORT=2023
MODE=udp


TMP="./test/temp.log"

TCP_IN=./test/tcp/in.log
TCP_OUT=./test/tcp/out.log

UDP_IN=./test/udp/in.log
UDP_OUT=./test/udp/out.log

while getopts ':h:p:m:' OPTION; do

  case "$OPTION" in
    h)
      HOST="$OPTARG"
      ;;

    p)
      PORT="$OPTARG"
      ;;

    m)
      MODE="$OPTARG"
      ;;

    ?)
      echo "Usage: $(basename $0) [-h AD.DR.ES.S] [-p PORT] [-m tcp/udp]"
      exit 1
      ;;
  esac

done

eval make
eval touch $TMP

if [ "$MODE" == "tcp" ];
then
cat $TCP_IN | ./ipkcpc -h $HOST -p $PORT -m $MODE > $TMP
eval diff $TMP $TCP_OUT
elif [ "$MODE" == "udp" ];
then
cat $UDP_IN | ./ipkcpc -h $HOST -p $PORT -m $MODE > $TMP &
sleep 1 #in some cases we have to wait for the output to be done, because it's udp.
PID=$!
kill -SIGINT $PID
eval diff $TMP $UDP_OUT
else
echo "Unknown mode"
exit 1
fi

eval rm $TMP
eval make clean