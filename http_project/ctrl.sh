#!/bin/bash

bin=http_server
port=8081
id=$(pidof $bin)

function start()
{
   if [ -z "$id" ];then
       ./$bin $port >> ./log/log.log
       echo "$id process running end ..."
   else
       echo "$bin is running!"
   fi
}

function stop()
{
   if [ -z "$id" ];then
       echo "$id process is not exists!"
   else
       kill -9 $id
       echo "$bin is end!"
       id=''
   fi

}
function status()
{
    if `pidof $bin > /dev/null`; then
        echo "status: running"
    else
        echo "status: dead"
    fi
}

case $1 in
    "start" )
        start
        ;;
    "stop" )
        stop
        ;;
    "restart" )
        stop
        start
        ;;
    "status" )
        status
        ;;
    * )
        echo "usage: $0 start | stop | status"
        ;;
esac
