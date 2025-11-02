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

typedef uint32_t index_t;

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

static void *get_elem_ptr(void *base, size_t size, index_t index)
{
    return (char *) base + (size * index);
}

static void sort_two(void *base, size_t size, index_t *index_array, compare_fn_t compare, void *context)
{
    const index_t a_index = index_array[0];
    const index_t b_index = index_array[1];
    void *a_ptr = get_elem_ptr(base, size, a_index);
    void *b_ptr = get_elem_ptr(base, size, b_index);
    const bool a_le_b = compare(a_ptr, b_ptr, context) <= 0;
    index_array[0] = a_le_b ? a_index : b_index;
    index_array[1] = a_le_b ? b_index : a_index;
}

static void sort_three(void *base, size_t size, index_t *index_array, compare_fn_t compare, void *context)
{
    const index_t a_index = index_array[0];
    const index_t b_index = index_array[1];
    const index_t c_index = index_array[2];
    void *a_ptr = get_elem_ptr(base, size, a_index);
    void *b_ptr = get_elem_ptr(base, size, b_index);
    void *c_ptr = get_elem_ptr(base, size, c_index);
    const bool a_le_b = compare(a_ptr, b_ptr, context) <= 0;
    const bool a_le_c = compare(a_ptr, c_ptr, context) <= 0;
    const bool b_le_c = compare(b_ptr, c_ptr, context) <= 0;
    const index_t min_a_c = a_le_c ? a_index : c_index;
    const index_t max_a_c = a_le_c ? c_index : a_index;
    const index_t min_b_c = b_le_c ? b_index : c_index;
    const index_t max_b_c = b_le_c ? c_index : b_index;
    index_array[0] = a_le_b ? min_a_c : min_b_c;
    index_array[1] = a_le_b ? (b_le_c ? b_index : max_a_c) : (a_le_c ? a_index : max_b_c);
    index_array[2] = a_le_b ? max_b_c : max_a_c;
}

static void merge_sort_rec(void *base, size_t nelems, size_t size, index_t *index_array, index_t *merge_array, compare_fn_t compare, void *context)
{
    if (nelems <= 2) {
        if (nelems == 2) {
            sort_two(base, size, index_array, compare, context);
        }
        return;
    } else if (nelems == 3) {
        sort_three(base, size, index_array, compare, context);
        return;
    }
    size_t lhs_nelems = nelems / 2;
    size_t rhs_nelems = nelems - lhs_nelems;
    merge_sort_rec(base, lhs_nelems, size, merge_array, index_array, compare, context);
    merge_sort_rec(base, rhs_nelems, size, merge_array + lhs_nelems, index_array + lhs_nelems, compare, context);
    index_t *lhs = merge_array;
    index_t *rhs = merge_array + lhs_nelems;
    index_t *lhs_end = lhs + lhs_nelems;
    index_t *rhs_end = rhs + rhs_nelems;
    index_t *dst = index_array;
    while (1) {
        const index_t lhs_index = *lhs;
        const index_t rhs_index = *rhs;
        const void *lhs_ptr = get_elem_ptr(base, size, lhs_index);
        const void *rhs_ptr = get_elem_ptr(base, size, rhs_index);
        const bool lhs_le_rhs = compare(lhs_ptr, rhs_ptr, context) <= 0;
        *dst++ = lhs_le_rhs ? lhs_index : rhs_index;
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

void merge_sort_indexed(void *base, size_t nelems, size_t size, compare_fn_t compare, void *context)
{
    index_t *index_array = calloc(nelems * 2, sizeof(index_t));
    index_t *merge_array = index_array + nelems;
    for (index_t i = 0; i < nelems; i++) {
        index_array[i] = i;
    }
    for (index_t i = 0; i < nelems; i++) {
        merge_array[i] = i;
    }
    merge_sort_rec(base, nelems, size, index_array, merge_array, compare, context);
    void *array_copy = calloc(nelems, size);
    copy(array_copy, base, nelems * size);
    char *dst_ptr = base;
    for (size_t i = 0; i < nelems; i++) {
        void *src_ptr = get_elem_ptr(array_copy, size, index_array[i]);
        copy(dst_ptr, src_ptr, size);
        dst_ptr += size;
    }
    free(array_copy);
    free(index_array);
}
