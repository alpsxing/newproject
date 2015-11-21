#!/bin/bash

export LD_LIBRARY_PATH=$PWD/lib/boost:$PWD/lib/openssl:$PWD/lib/sqlite

./new_project --debug 0

