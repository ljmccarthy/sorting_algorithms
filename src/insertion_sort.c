#include <stdlib.h>
#include <string.h>
#include "sort.h"

void insertion_sort(void *base, size_t nelems, size_t size, compare_fn_t compare, void *context)
{
    char temp_buf[1024];
    char *temp = size > sizeof(temp_buf) ? malloc(size) : temp_buf;
    char *end = (char *) base + nelems * size;
    for (char *unsorted = (char *) base + size; unsorted != end; unsorted += size) {
        for (char *cur = unsorted; cur != base && compare(cur - size, cur, context) > 0; cur -= size) {
            memcpy(temp, cur - size, size);
            memcpy(cur - size, cur, size);
            memcpy(cur, temp, size);
        }
    }
    if (temp != temp_buf) {
        free(temp);
    }
}
