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
#include <stdlib.h>
#include "sort.h"
#include "util.h"

static void sort_two(char *array, char *merge_array, size_t size, compare_fn_t compare, void *context)
{
    void *a = merge_array + 0 * size;
    void *b = merge_array + 1 * size;
    copy(a, array + 0 * size, size);
    copy(b, array + 1 * size, size);
    bool a_le_b = compare(a, b, context) <= 0;
    copy(array + 0 * size, a_le_b ? a : b, size);
    copy(array + 1 * size, a_le_b ? b : a, size);
}

static void sort_three(char *array, char *merge_array, size_t size, compare_fn_t compare, void *context)
{
    void *a = merge_array + 0 * size;
    void *b = merge_array + 1 * size;
    void *c = merge_array + 2 * size;
    copy(a, array + 0 * size, size);
    copy(b, array + 1 * size, size);
    copy(c, array + 2 * size, size);
    bool a_le_b = compare(a, b, context) <= 0;
    bool a_le_c = compare(a, c, context) <= 0;
    bool b_le_c = compare(b, c, context) <= 0;
    void *min_a_c = a_le_c ? a : c;
    void *max_a_c = a_le_c ? c : a;
    void *min_b_c = b_le_c ? b : c;
    void *max_b_c = b_le_c ? c : b;
    copy(array + 0 * size, a_le_b ? min_a_c : min_b_c, size);
    copy(array + 1 * size, a_le_b ? (b_le_c ? b : max_a_c) : (a_le_c ? a : max_b_c), size);
    copy(array + 2 * size, a_le_b ? max_b_c : max_a_c, size);
}

static void merge_sort_rec(char *array, char *merge_array, size_t nelems, size_t size, compare_fn_t compare, void *context)
{
    if (nelems <= 2) {
        if (nelems == 2) {
            sort_two(array, merge_array, size, compare, context);
        }
        return;
    } else if (nelems == 3) {
        sort_three(array, merge_array, size, compare, context);
        return;
    }
    size_t lhs_nelems = nelems / 2;
    size_t rhs_nelems = nelems - lhs_nelems;
    merge_sort_rec(merge_array, array, lhs_nelems, size, compare, context);
    merge_sort_rec(merge_array + lhs_nelems * size, array + lhs_nelems * size, rhs_nelems, size, compare, context);
    char *lhs = merge_array;
    char *rhs = merge_array + lhs_nelems * size;
    char *lhs_end = rhs;
    char *rhs_end = rhs + rhs_nelems * size;
    char *dst = array;
    while (1) {
        bool lhs_le_rhs = compare(lhs, rhs, context) <= 0;
        copy(dst, lhs_le_rhs ? lhs : rhs, size);
        dst += size;
        lhs += lhs_le_rhs ? size : 0;
        rhs += lhs_le_rhs ? 0 : size;
        if (unlikely(lhs == lhs_end)) {
            copy(dst, rhs, (size_t) (rhs_end - rhs));
            break;
        }
        if (unlikely(rhs == rhs_end)) {
            copy(dst, lhs, (size_t) (lhs_end - lhs));
            break;
        }
    }
}

void merge_sort(void *base, size_t nelems, size_t size, compare_fn_t compare, void *context)
{
    size_t array_size = nelems * size;
    char *merge_array = malloc(array_size);
    copy(merge_array, base, array_size);
    merge_sort_rec(base, merge_array, nelems, size, compare, context);
    free(merge_array);
}
