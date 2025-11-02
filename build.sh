#!/bin/bash
set -euo pipefail

usage() {
    echo "Usage: $0 (Debug|Release)"
    exit 1
}

if [ $# -ne 1 ]; then
    usage
fi

BUILD_TYPE="$1"
shift
if [ "$BUILD_TYPE" = Debug ]; then
    BUILD_TYPE_CFLAGS="-O0 -ggdb -fsanitize=address -fsanitize=undefined"
elif [ "$BUILD_TYPE" = Release ]; then
    BUILD_TYPE_CFLAGS="-O2 -DNDEBUG"
else
    usage
fi

if [ -z "${CC+x}" ]; then
    CC=cc
fi

PLATFORM_CFLAGS=""
if [ "$(uname)" = Linux ]; then
    PLATFORM_CFLAGS="-DLIBBSD_OVERLAY -isystem /usr/include/bsd -lbsd"
fi

BUILD_DIR="build/$BUILD_TYPE"
CFLAGS="$BUILD_TYPE_CFLAGS $PLATFORM_CFLAGS -std=c17 -Wall -Wextra -Wpedantic -Wconversion -Wstrict-overflow=5"

mkdir -p "$BUILD_DIR"
$CC $CFLAGS -o "$BUILD_DIR/test_sort" src/*.c third_party/*.c
