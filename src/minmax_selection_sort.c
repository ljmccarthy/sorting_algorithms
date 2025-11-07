#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include "sort.h"
#include "util.h"

void minmax_selection_sort(void *base, size_t nelems, size_t size, compare_fn_t compare, void *context)
{
    char temp_buf[1024];
    if (nelems <= 1) return;
    char *temp = size > sizeof(temp_buf) ? malloc(size) : temp_buf;
    char *min_pos = base;
    char *max_pos = min_pos + (nelems - 1) * size;
    while (min_pos < max_pos) {
        char *min_found = min_pos;
        char *max_found = min_pos;
        char *end_pos = max_pos + size;
        for (char *elem = min_pos + size; elem != end_pos; elem += size) {
            min_found = compare(elem, min_found, context) < 0 ? elem : min_found;
            max_found = compare(elem, max_found, context) > 0 ? elem : max_found;
        }
        swap(min_pos, min_found, temp, size);
        max_found = (max_found == min_pos) ? min_found : max_found;
        swap(max_pos, max_found, temp, size);
        min_pos += size;
        max_pos -= size;
    }
    if (temp != temp_buf) {
        free(temp);
    }
}
