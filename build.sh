#!/bin/bash
#
# Written by Luke McCarthy <luke@iogopro.co.uk>
# https://github.com/ljmccarthy/sorting_algorithms
#
# This is free and unencumbered software released into the public domain.
#
# Anyone is free to copy, modify, publish, use, compile, sell, or
# distribute this software, either in source code form or as a compiled
# binary, for any purpose, commercial or non-commercial, and by any
# means.
#
# In jurisdictions that recognize copyright laws, the author or authors
# of this software dedicate any and all copyright interest in the
# software to the public domain. We make this dedication for the benefit
# of the public at large and to the detriment of our heirs and
# successors. We intend this dedication to be an overt act of
# relinquishment in perpetuity of all present and future rights to this
# software under copyright law.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# For more information, please refer to <https://unlicense.org/>

CFLAGS="-std=c17 -Wall -Wextra -Wpedantic -Wconversion -Wstrict-overflow=5 -Wno-missing-field-initializers"
CFLAGS_Debug="-O0 -ggdb -fsanitize=address -fsanitize=undefined"
CFLAGS_Release="-O2 -DNDEBUG"

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
if [ "$BUILD_TYPE" != Debug -a "$BUILD_TYPE" != Release ]; then
    usage
fi

if [ -z "${CC+x}" ]; then
    CC=cc
fi

PLATFORM_CFLAGS=""
if [ "$(uname)" = Linux -a -d /usr/include/bsd ]; then
    PLATFORM_CFLAGS="-DLIBBSD_OVERLAY -isystem /usr/include/bsd -lbsd"
fi

BUILD_DIR="build/$BUILD_TYPE"
CFLAGS_VARIANT_VAR="CFLAGS_${BUILD_TYPE}"
CFLAGS="${!CFLAGS_VARIANT_VAR} $PLATFORM_CFLAGS $CFLAGS"

mkdir -p "$BUILD_DIR"
$CC $CFLAGS -o "$BUILD_DIR/test_sort" src/*.c third_party/*.c third_party/*/*.c
