/*
 * Written by Luke McCarthy <luke@iogopro.co.uk>
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

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "sort.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

typedef int (*compare_without_context_fn_t)(const void *lhs, const void *rhs);
typedef int (*compare_with_context_first_fn_t)(void *context, const void *lhs, const void *rhs);
typedef int (*compare_with_context_last_fn_t)(const void *lhs, const void *rhs, void *context);

enum sort_function_type {
    SORT_FN_NO_CONTEXT,
    SORT_FN_INT_NO_CONTEXT,
    SORT_FN_COMPARE_WITH_CONTEXT_FIRST_THEN_CONTEXT,
    SORT_FN_COMPARE_WITH_CONTEXT_LAST_THEN_CONTEXT,
    SORT_FN_CONTEXT_THEN_COMPARE_WITH_CONTEXT_FIRST,
    SORT_FN_CONTEXT_THEN_COMPARE_WITH_CONTEXT_LAST,
};

struct sort_function {
    const char *name;
    enum sort_function_type type;
    union {
        void (*no_context)(void *base, size_t nelems, size_t size, compare_without_context_fn_t compare);
        int (*int_no_context)(void *base, size_t nelems, size_t size, compare_without_context_fn_t compare);
        void (*compare_with_context_first_then_context)(void *base, size_t nelems, size_t size, compare_with_context_first_fn_t compare, void *context);
        void (*compare_with_context_last_then_context)(void *base, size_t nelems, size_t size, compare_with_context_last_fn_t compare, void *context);
        void (*context_then_compare_with_context_first)(void *base, size_t nelems, size_t size, void *context, compare_with_context_first_fn_t compare);
        void (*context_then_compare_with_context_last)(void *base, size_t nelems, size_t size, void *context, compare_with_context_last_fn_t compare);
    } fn;
};

typedef struct sort_function sort_fn_t;

static const sort_fn_t sort_functions[] = {
    {"qsort", SORT_FN_NO_CONTEXT, {.no_context = qsort}},
    {"mergesort", SORT_FN_INT_NO_CONTEXT, {.int_no_context = mergesort}},
    {"heapsort", SORT_FN_INT_NO_CONTEXT, {.int_no_context = heapsort}},
#if defined(__APPLE__)
    {"psort", SORT_FN_NO_CONTEXT, {.no_context = psort}},
#endif
    {"smoothsort", SORT_FN_COMPARE_WITH_CONTEXT_LAST_THEN_CONTEXT, {.compare_with_context_last_then_context = smoothsort}},
    {"merge_sort", SORT_FN_COMPARE_WITH_CONTEXT_LAST_THEN_CONTEXT, {.compare_with_context_last_then_context = merge_sort}},
    {"merge_sort_ptr", SORT_FN_COMPARE_WITH_CONTEXT_LAST_THEN_CONTEXT, {.compare_with_context_last_then_context = merge_sort_ptr}},
    {"merge_sort_indexed", SORT_FN_COMPARE_WITH_CONTEXT_LAST_THEN_CONTEXT, {.compare_with_context_last_then_context = merge_sort_indexed}},
};

static int compare_elem(const void *a_ptr, const void *b_ptr)
{
    size_t a, b;
    memcpy(&a, a_ptr, sizeof(size_t));
    memcpy(&b, b_ptr, sizeof(size_t));
    return (a > b) - (a < b);
}

static int compare_elem_with_context(void *context, const void *a_ptr, const void *b_ptr)
{
    (void) context;
    return compare_elem(a_ptr, b_ptr);
}

static int compare_elem_with_context_last(const void *a_ptr, const void *b_ptr, void *context)
{
    (void) context;
    return compare_elem(a_ptr, b_ptr);
}

static int call_sort_function(const sort_fn_t *sort, void *base, size_t nelems, size_t size, void *context)
{
    int result = 0;
    switch (sort->type) {
        case SORT_FN_NO_CONTEXT:
            sort->fn.no_context(base, nelems, size, compare_elem);
            break;
        case SORT_FN_INT_NO_CONTEXT:
            result = sort->fn.int_no_context(base, nelems, size, compare_elem);
            break;
        case SORT_FN_COMPARE_WITH_CONTEXT_FIRST_THEN_CONTEXT:
            sort->fn.compare_with_context_first_then_context(base, nelems, size, compare_elem_with_context, context);
            break;
        case SORT_FN_COMPARE_WITH_CONTEXT_LAST_THEN_CONTEXT:
            sort->fn.compare_with_context_last_then_context(base, nelems, size, compare_elem_with_context_last, context);
            break;
        case SORT_FN_CONTEXT_THEN_COMPARE_WITH_CONTEXT_FIRST:
            sort->fn.context_then_compare_with_context_first(base, nelems, size, context, compare_elem_with_context);
            break;
        case SORT_FN_CONTEXT_THEN_COMPARE_WITH_CONTEXT_LAST:
            sort->fn.context_then_compare_with_context_last(base, nelems, size, context, compare_elem_with_context_last);
            break;
        default:
            assert(0 && "unknown sort function type");
    }
    return result;
}

static void print_array(char *array, size_t nelems, size_t size)
{
    printf("[\n");
    for (size_t i = 0; i < nelems; i++) {
        size_t key;
        memcpy(&key, array + i * size, sizeof(size_t));
        printf("  %zu", key);
        if (i < nelems - 1) {
            printf(",\n");
        }
    }
    printf("\n]\n");
}

static bool test_sort(void *array, size_t size, size_t nelems, const sort_fn_t *sort, const char *test_name, clock_t *out_time)
{
    printf("\r\x1b[K> Testing %s...", test_name);
    fflush(stdout);
    void *array_copy_test = calloc(nelems, size);
    void *array_copy_check = calloc(nelems, size);
    memcpy(array_copy_test, array, nelems * size);
    memcpy(array_copy_check, array, nelems * size);
    clock_t start_time = clock();
    call_sort_function(sort, array_copy_test, nelems, size, NULL);
    *out_time = clock() - start_time;
    qsort(array_copy_check, nelems, size, compare_elem);
    bool result = (memcmp(array_copy_test, array_copy_check, nelems * size) == 0);
    if (!result) {
        printf("\nArray after sort:\n");
        print_array(array_copy_test, nelems, size);
        printf("Test '%s' failed!\n", test_name);
    }
    free(array_copy_test);
    free(array_copy_check);
    if (result) {
        printf("\r\x1b[K");
    }
    return result;
}

// LCG algorithm
typedef uint32_t random_seed_t;
static inline uint32_t random_uint32(random_seed_t *seed)
{
    *seed = 1664525 * (*seed) + 1013904223;
    return *seed;
}

static void init_array_sequential(char *array, size_t array_size, size_t elem_size)
{
    char *elem = array;
    for (size_t i = 0; i < array_size; i++) {
        memset(elem, 0, elem_size);
        memcpy(elem, &i, sizeof(size_t));
        elem += elem_size;
    }
}

static void reverse_array(char *array, size_t array_size, size_t elem_size)
{
    void *temp = malloc(elem_size);
    for (size_t i = 0; i < array_size / 2; i++) {
        memcpy(temp, array + i * elem_size, elem_size);
        memcpy(array + i * elem_size, array + (array_size - 1 - i) * elem_size, elem_size);
        memcpy(array + (array_size - 1 - i) * elem_size, temp, elem_size);
    }
    free(temp);
}

static void random_shuffle(char *array, size_t array_size, size_t elem_size, uint32_t *seed)
{
    void *temp = malloc(elem_size);
    for (size_t i = 0; i < array_size; i++) {
        size_t j = random_uint32(seed) % (i + 1);
        memcpy(temp, array + i * elem_size, elem_size);
        memcpy(array + i * elem_size, array + j * elem_size, elem_size);
        memcpy(array + j * elem_size, temp, elem_size);
    }
    free(temp);
}

static bool test_ordered_array(const sort_fn_t *sort, size_t array_size, size_t elem_size, clock_t *out_time)
{
    char *array = calloc(array_size, elem_size);
    init_array_sequential(array, array_size, elem_size);
    bool result = test_sort(array, elem_size, array_size, sort, "ordered array", out_time);
    free(array);
    return result;
}

static bool test_mostly_ordered_array(const sort_fn_t *sort, size_t array_size, size_t elem_size, random_seed_t *seed, clock_t *out_time)
{
    char *array = calloc(array_size, elem_size);
    init_array_sequential(array, array_size, elem_size);
    char *temp = malloc(elem_size);
    for (size_t i = 0; i < array_size / 10; i++) {
        size_t j = random_uint32(seed) % array_size;
        memcpy(temp, array + i * elem_size, elem_size);
        memcpy(array + i * elem_size, array + j * elem_size, elem_size);
        memcpy(array + j * elem_size, temp, elem_size);
    }
    free(temp);
    bool result = test_sort(array, elem_size, array_size, sort, "mostly ordered array", out_time);
    free(array);
    return result;
}

static bool test_reverse_ordered_array(const sort_fn_t *sort, size_t array_size, size_t elem_size, clock_t *out_time)
{
    char *array = calloc(array_size, elem_size);
    init_array_sequential(array, array_size, elem_size);
    reverse_array(array, array_size, elem_size);
    bool result = test_sort(array, elem_size, array_size, sort, "reverse ordered array", out_time);
    free(array);
    return result;
}

static bool test_random_array(const sort_fn_t *sort, size_t array_size, size_t elem_size, random_seed_t *seed, clock_t *out_time)
{
    char *array = calloc(array_size, elem_size);
    init_array_sequential(array, array_size, elem_size);
    random_shuffle(array, array_size, elem_size, seed);
    bool result = test_sort(array, elem_size, array_size, sort, "random array", out_time);
    free(array);
    return result;
}

static bool run_tests(const sort_fn_t *sort, random_seed_t *seed, size_t array_size, size_t elem_size)
{
    printf("Testing sort function: %s\n", sort->name);

    clock_t times[4];
    if (!test_ordered_array(sort, array_size, elem_size, &times[0])) {
        return false;
    }
    if (!test_reverse_ordered_array(sort, array_size, elem_size, &times[1])) {
        return false;
    }
    if (!test_mostly_ordered_array(sort, array_size, elem_size, seed, &times[2])) {
        return false;
    }
    if (!test_random_array(sort, array_size, elem_size, seed, &times[3])) {
        return false;
    }

    // Don't print timing information in debug builds to avoid unfair comparisons
#ifdef NDEBUG
    clock_t total_time = 0;
    for (size_t i = 0; i < ARRAY_SIZE(times); i++) {
        total_time += times[i];
    }

    double total_time_seconds = (double) total_time / CLOCKS_PER_SEC;
    printf("Time: %.2f seconds\n", total_time_seconds);
#endif

    return true;
}

static void usage(void)
{
    printf("usage: test_sort [-f <function>] [-n <array-size>] [-s <elem-size>] [-r <seed>]\n");
    printf("available sort functions:\n");
    for (size_t i = 0; i < ARRAY_SIZE(sort_functions); i++) {
        printf("    %s\n", sort_functions[i].name);
    }
}

int main(int argc, char **argv)
{
    const sort_fn_t *sort = NULL;
    size_t array_size = 1000000;
    size_t elem_size = 64;
    random_seed_t seed = 0xCAFECAFE;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage();
            return 0;
        } else if (strcmp(argv[i], "-f") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "error: missing argument to -f\n");
                usage();
                return 1;
            }
            const char *sort_name = argv[++i];
            for (size_t j = 0; j < ARRAY_SIZE(sort_functions); j++) {
                if (strcmp(sort_name, sort_functions[j].name) == 0) {
                    sort = &sort_functions[j];
                    break;
                }
            }
            if (!sort) {
                fprintf(stderr, "error: unknown sort function: %s\n", sort_name);
                usage();
            }
        } else if (strcmp(argv[i], "-n") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "error: missing argument to -n\n");
                usage();
                return 1;
            }
            unsigned long long size = strtoull(argv[++i], NULL, 10);
            if (size <= 0 || size != (size_t) size) {
                fprintf(stderr, "error: invalid array size: %llu\n", size);
                usage();
                return 1;
            }
            array_size = (size_t) size;
        } else if (strcmp(argv[i], "-s") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "error: missing argument to -s\n");
                usage();
                return 1;
            }
            unsigned long long size = strtoull(argv[++i], NULL, 10);
            if (size <= 0 || size < sizeof(size_t) || size != (size_t) size) {
                fprintf(stderr, "error: invalid element size: %llu\n", size);
                usage();
                return 1;
            }
            elem_size = (size_t) size;
        } else if (strcmp(argv[i], "-r") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "error: missing argument to -r\n");
                usage();
                return 1;
            }
            seed = (random_seed_t) strtoul(argv[++i], NULL, 10);
        } else {
            fprintf(stderr, "error: unknown argument: %s\n", argv[i]);
            usage();
            return 1;
        }
    }

    printf("Array size: %zu, Element size: %zu, Random seed: %u\n", array_size, elem_size, seed);
    if (!sort) {
        for (size_t i = 0; i < ARRAY_SIZE(sort_functions); i++) {
            if (!run_tests(&sort_functions[i], &seed, array_size, elem_size)) {
                return 1;
            }
        }
    } else {
        if (!run_tests(sort, &seed, array_size, elem_size)) {
            return 1;
        }
    }
    printf("All tests passed.\n");
    return 0;
}
