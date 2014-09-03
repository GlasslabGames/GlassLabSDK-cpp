#!/bin/bash
export LIB_NAME="libevent"
export LIB_OUTPUT="${LIB_NAME}.a ${LIB_NAME}_core.a ${LIB_NAME}_extra.a ${LIB_NAME}_openssl.a ${LIB_NAME}_pthreads.a"
export LIB_FILE_PREFIX=$LIB_NAME
export LIB_VERSION="2.0.21-stable"
export LIB_URL="https://github.com/downloads/libevent/libevent/${LIB_FILE_PREFIX}-${LIB_VERSION}.tar.gz"

./build-lib.sh
