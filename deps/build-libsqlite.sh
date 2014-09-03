#!/bin/bash
export LIB_NAME="libsqlite"
export LIB_OUTPUT="${LIB_NAME}3.a"
export LIB_FILE_PREFIX="sqlite-autoconf"
export LIB_VERSION="3080301"
export LIB_URL="http://www.sqlite.org/2014/${LIB_FILE_PREFIX}-${LIB_VERSION}.tar.gz"

./build-lib.sh
