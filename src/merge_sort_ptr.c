/*
 * Written by Luke McCarthy <luke@iogopro.co.uk>, Oct-Nov 2025
 * https://github.com/ljmccarthy/sorting_algorithms
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <https://unlicense.org/>
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "sort.h"

#if defined(__GNUC__) || defined(__clang__)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define unlikely(x) (x)
#endif

#if defined(__APPLE__)
static void copy(void *dst_ptr, const void *src_ptr, size_t size)
{
    char *dst = dst_ptr;
    const char *src = src_ptr;
    char *dst_end = dst + size;
    do { *dst++ = *src++; } while (dst != dst_end);
}
#else
#include <string.h>
static void copy(void *dst_ptr, const void *src_ptr, size_t size)
{
    memcpy(dst_ptr, src_ptr, size);
}
#endif

static void sort_two(void **ptr_array, compare_fn_t compare, void *context)
{
    void *a_ptr = ptr_array[0];
    void *b_ptr = ptr_array[1];
    bool a_le_b = compare(a_ptr, b_ptr, context) <= 0;
    ptr_array[0] = a_le_b ? a_ptr : b_ptr;
    ptr_array[1] = a_le_b ? b_ptr : a_ptr;
}

static void sort_three(void **ptr_array, compare_fn_t compare, void *context)
{
    void *a_ptr = ptr_array[0];
    void *b_ptr = ptr_array[1];
    void *c_ptr = ptr_array[2];
    bool a_le_b = compare(a_ptr, b_ptr, context) <= 0;
    bool a_le_c = compare(a_ptr, c_ptr, context) <= 0;
    bool b_le_c = compare(b_ptr, c_ptr, context) <= 0;
    void *min_a_c = a_le_c ? a_ptr : c_ptr;
    void *max_a_c = a_le_c ? c_ptr : a_ptr;
    void *min_b_c = b_le_c ? b_ptr : c_ptr;
    void *max_b_c = b_le_c ? c_ptr : b_ptr;
    ptr_array[0] = a_le_b ? min_a_c : min_b_c;
    ptr_array[1] = a_le_b ? (b_le_c ? b_ptr : max_a_c) : (a_le_c ? a_ptr : max_b_c);
    ptr_array[2] = a_le_b ? max_b_c : max_a_c;
}

static void merge_sort_rec(void **ptr_array, void **merge_ptr_array, size_t nelems, size_t size, compare_fn_t compare, void *context)
{
    if (nelems <= 2) {
        if (nelems == 2) {
            sort_two(ptr_array, compare, context);
        }
        return;
    } else if (nelems == 3) {
        sort_three(ptr_array, compare, context);
        return;
    }
    size_t lhs_nelems = nelems / 2;
    size_t rhs_nelems = nelems - lhs_nelems;
    merge_sort_rec(merge_ptr_array, ptr_array, lhs_nelems, size, compare, context);
    merge_sort_rec(merge_ptr_array + lhs_nelems, ptr_array + lhs_nelems, rhs_nelems, size, compare, context);
    void **lhs = merge_ptr_array;
    void **rhs = merge_ptr_array + lhs_nelems;
    void **lhs_end = lhs + lhs_nelems;
    void **rhs_end = rhs + rhs_nelems;
    void **dst = ptr_array;
    while (1) {
        void *lhs_ptr = *lhs;
        void *rhs_ptr = *rhs;
        bool lhs_le_rhs = compare(lhs_ptr, rhs_ptr, context) <= 0;
        *dst++ = lhs_le_rhs ? lhs_ptr : rhs_ptr;
        lhs += lhs_le_rhs;
        rhs += !lhs_le_rhs;
        if (unlikely(lhs == lhs_end)) {
            copy(dst, rhs, (size_t) ((char *) rhs_end - (char *) rhs));
            break;
        }
        if (unlikely(rhs == rhs_end)) {
            copy(dst, lhs, (size_t) ((char *) lhs_end - (char *) lhs));
            break;
        }
    }
}

void merge_sort_ptr(void *base, size_t nelems, size_t size, compare_fn_t compare, void *context)
{
    void *elem_array = calloc(nelems, size);
    copy(elem_array, base, nelems * size);
    void **ptr_array = calloc(nelems * 2, sizeof(void *));
    void **merge_ptr_array = ptr_array + nelems;
    char *elem_ptr = elem_array;
    for (size_t i = 0; i < nelems; i++) {
        ptr_array[i] = elem_ptr;
        merge_ptr_array[i] = elem_ptr;
        elem_ptr += size;
    }
    merge_sort_rec(ptr_array, merge_ptr_array, nelems, size, compare, context);
    char *dst_ptr = base;
    for (size_t i = 0; i < nelems; i++) {
        copy(dst_ptr, ptr_array[i], size);
        dst_ptr += size;
    }
    free(elem_array);
    free(ptr_array);
}
