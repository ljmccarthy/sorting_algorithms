/*
 * Written by Luke McCarthy <luke@iogopro.co.uk>, 7 Nov 2025
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

 #include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include "sort.h"
#include "util.h"

void selection_sort(void *base, size_t nelems, size_t size, compare_fn_t compare, void *context)
{
    char temp_buf[1024];
    if (nelems <= 1) return;
    char *temp = size > sizeof(temp_buf) ? malloc(size) : temp_buf;
    char *min_pos = base;
    char *end_pos = min_pos + nelems * size;
    while (min_pos != end_pos) {
        char *min_found = min_pos;
        for (char *elem = min_pos + size; elem != end_pos; elem += size) {
            min_found = compare(elem, min_found, context) < 0 ? elem : min_found;
        }
        swap(min_pos, min_found, temp, size);
        min_pos += size;
    }
    if (temp != temp_buf) {
        free(temp);
    }
}
