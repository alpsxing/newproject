#!/bin/bash

export LD_LIBRARY_PATH=$PWD/lib/boost:$PWD/lib/openssl:$PWD/lib/sqlite::$PWD/lib/aircrack
export WIRELESS_NETWORK_IFACE=mon0

./new_project --debug 0

