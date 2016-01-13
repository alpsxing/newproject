#!/bin/bash

export LD_LIBRARY_PATH=$PWD/lib/boost:$PWD/lib/openssl:$PWD/lib/sqlite::$PWD/lib/aircrack:$PWD/lib/libnl
export WIRELESS_NETWORK_IFACE=mon0

./new_project --debug 0

