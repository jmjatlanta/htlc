#!/bin/bash
DATADIR="./blockchain/"

PIDFILE=$DATADIR"/nodeos.pid"

if [ -f $PIDFILE ]; then
   pid=`cat $PIDFILE`
   echo $pid
   kill $pid
   rm -r $PIDFILE

   echo -ne "Stoping Node"
   while true; do
      [ ! -d "/proc/$pid/fd" ] && break
      echo -ne "."
      sleep 1
   done

   echo -ne "\rNode Stopped. \n"
fi

PIDFILE=$DATADIR"/keosd.pid"

if [ -f $PIDFILE ]; then
   pid=`cat $PIDFILE`
   echo $pid
   kill $pid
   rm -r $PIDFILE

   echo -ne "Stoping key daemon"
   while true; do
      [ ! -d "/proc/$pid/fd" ] && break
      echo -ne "."
      sleep 1
   done

   echo -ne "\rkey deamon Stopped. \n"
fi

