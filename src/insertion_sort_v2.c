/*
 * Written by Luke McCarthy <luke@iogopro.co.uk>, 4 Nov 2025
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

#include <stdlib.h>
#include <string.h>
#include "sort.h"

/*
 * This version of insertion sort first determines the correct position to insert before
 * moving any data, then uses memmove to bulk move all elements after the insertion point.
 * This is faster than repeatedly swapping the element until it finds the correct position.
 * It also scans the sorted array forwards instead of backwards.
 */
void insertion_sort_v2(void *base, size_t nelems, size_t size, compare_fn_t compare, void *context)
{
    char temp_buf[1024];
    char *temp = size > sizeof(temp_buf) ? malloc(size) : temp_buf;
    char *end = (char *) base + nelems * size;
    for (char *unsorted = (char *) base + size; unsorted != end; unsorted += size) {
        for (char *cur = base; cur != unsorted; cur += size) {
            if (compare(unsorted, cur, context) < 0) {
                memcpy(temp, unsorted, size);
                memmove(cur + size, cur, (size_t) (unsorted - cur));
                memcpy(cur, temp, size);
                break;
            }
        }
    }
    if (temp != temp_buf) {
        free(temp);
    }
}
