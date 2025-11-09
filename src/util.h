/*
 * Written by Luke McCarthy <luke@iogopro.co.uk>
 * https://github.com/ljmccarthy/sorting_algorithms
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or distribute
 * this software, either in source code form or as a compiled binary, for any
 * purpose, commercial or non-commercial, and by any means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors of
 * this software dedicate any and all copyright interest in the software to the
 * public domain. We make this dedication for the benefit of the public at
 * large and to the detriment of our heirs and successors. We intend this
 * dedication to be an overt act of relinquishment in perpetuity of all present
 * and future rights to this software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <https://unlicense.org/>
 */

#pragma once
#include <stddef.h>
#include <string.h>

#if defined(__GNUC__) || defined(__clang__)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define unlikely(x) (x)
#endif

static inline void copy(void *dst_ptr, const void *src_ptr, size_t size)
{
#if defined(__APPLE__)
    /* for some unknown reason this is faster than calling memcpy on Apple Silicon Macs */
    char *dst = dst_ptr;
    const char *src = src_ptr;
    char *dst_end = dst + size;
    do { *dst++ = *src++; } while (dst != dst_end);
#else
    memcpy(dst_ptr, src_ptr, size);
#endif
}

static inline void swap(void *a_ptr, void *b_ptr, void *temp, size_t size)
{
    copy(temp, a_ptr, size);
    copy(a_ptr, b_ptr, size);
    copy(b_ptr, temp, size);
}
