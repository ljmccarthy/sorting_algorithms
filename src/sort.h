#pragma once
#include <stddef.h>

typedef int (*compare_fn_t)(const void *, const void *, void *);

/* Our implementations */
void merge_sort(void *base, size_t nelems, size_t size, compare_fn_t compare, void *context);
void merge_sort_ptr(void *base, size_t nelems, size_t size, compare_fn_t compare, void *context);
void merge_sort_indexed(void *base, size_t nelems, size_t size, compare_fn_t compare, void *context);

/* Third-party sorting algorithms */
void smoothsort(void *base, size_t nelems, size_t size, compare_fn_t compare, void *context);
