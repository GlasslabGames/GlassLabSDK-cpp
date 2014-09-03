#!/bin/bash
export LIB_FILE_PREFIX="jansson"
export LIB_NAME="lib${LIB_FILE_PREFIX}"
export LIB_OUTPUT="${LIB_NAME}.a"
export LIB_VERSION="2.5"
export LIB_URL="http://www.digip.org/jansson/releases/${LIB_FILE_PREFIX}-${LIB_VERSION}.tar.gz"

./build-lib.sh
