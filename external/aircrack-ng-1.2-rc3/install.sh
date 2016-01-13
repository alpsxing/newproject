#!/bin/bash

echo "./install.sh  installdir"
if [ ! -d $1 ]; then
    mkdir -p $1
	mkdir $1/include
	mkdir $1/lib
fi

if [ ! -d $1/lib ]; then
	mkdir $1/lib
fi

if [ ! -d $1/include ]; then
	mkdir $1/include
fi

cp -arf src/libairodump-ng.so* $1/lib
cp src/airodump-ng.h	$1/include
cp src/pcap.h 	$1/include

echo "ok"


