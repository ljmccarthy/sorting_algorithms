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
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "sort.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define ELEM_MAX UINT32_MAX

typedef uint32_t elem_t;

typedef int (*compare_without_context_fn_t)(const void *lhs, const void *rhs);
typedef int (*compare_with_context_first_fn_t)(void *context, const void *lhs, const void *rhs);
typedef int (*compare_with_context_last_fn_t)(const void *lhs, const void *rhs, void *context);

enum sort_function_type {
    SORT_FN_VOID_NO_CONTEXT,
    SORT_FN_INT_NO_CONTEXT,
    SORT_FN_VOID_COMPARE_WITH_CONTEXT_FIRST_THEN_CONTEXT,
    SORT_FN_VOID_COMPARE_WITH_CONTEXT_LAST_THEN_CONTEXT,
    SORT_FN_INT_COMPARE_WITH_CONTEXT_LAST_THEN_CONTEXT,
    SORT_FN_VOID_CONTEXT_THEN_COMPARE_WITH_CONTEXT_FIRST,
    SORT_FN_VOID_CONTEXT_THEN_COMPARE_WITH_CONTEXT_LAST,
    SORT_FN_INT_CONTEXT_THEN_COMPARE_WITH_CONTEXT_LAST,
};

enum performance {
    PERF_SLOW,
    PERF_MID,
    PERF_FAST,
};

struct sort_function {
    const char *name;
    enum sort_function_type type;
    union {
        void (*void_no_context)(void *base, size_t nelems, size_t size, compare_without_context_fn_t compare);
        int (*int_no_context)(void *base, size_t nelems, size_t size, compare_without_context_fn_t compare);
        void (*void_compare_with_context_first_then_context)(void *base, size_t nelems, size_t size, compare_with_context_first_fn_t compare, void *context);
        void (*void_compare_with_context_last_then_context)(void *base, size_t nelems, size_t size, compare_with_context_last_fn_t compare, void *context);
        int (*int_compare_with_context_last_then_context)(void *base, size_t nelems, size_t size, compare_with_context_last_fn_t compare, void *context);
        void (*void_context_then_compare_with_context_first)(void *base, size_t nelems, size_t size, void *context, compare_with_context_first_fn_t compare);
        void (*void_context_then_compare_with_context_last)(void *base, size_t nelems, size_t size, void *context, compare_with_context_last_fn_t compare);
    } fn;
    enum performance perf;
};

typedef struct sort_function sort_fn_t;

static const sort_fn_t sort_functions[] = {
    /* system-provided sort functions */
    {"qsort", SORT_FN_VOID_NO_CONTEXT, {.void_no_context = qsort}, .perf = PERF_FAST},
#if defined(LIBBSD_OVERLAY) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
    {"mergesort", SORT_FN_INT_NO_CONTEXT, {.int_no_context = mergesort}, .perf = PERF_FAST},
    {"heapsort", SORT_FN_INT_NO_CONTEXT, {.int_no_context = heapsort}, .perf = PERF_MID},
#endif
#if defined(__APPLE__)
    {"psort", SORT_FN_VOID_NO_CONTEXT, {.void_no_context = psort}, .perf = PERF_FAST},
#endif
    /* our implementations */
    {"merge_sort", SORT_FN_VOID_COMPARE_WITH_CONTEXT_LAST_THEN_CONTEXT, {.void_compare_with_context_last_then_context = merge_sort}, .perf = PERF_FAST},
    {"merge_sort_ptr", SORT_FN_VOID_COMPARE_WITH_CONTEXT_LAST_THEN_CONTEXT, {.void_compare_with_context_last_then_context = merge_sort_ptr}, .perf = PERF_FAST},
    {"merge_sort_indexed", SORT_FN_VOID_COMPARE_WITH_CONTEXT_LAST_THEN_CONTEXT, {.void_compare_with_context_last_then_context = merge_sort_indexed}, .perf = PERF_FAST},
    {"insertion_sort", SORT_FN_VOID_COMPARE_WITH_CONTEXT_LAST_THEN_CONTEXT, {.void_compare_with_context_last_then_context = insertion_sort}, .perf = PERF_SLOW},
    {"insertion_sort_v2", SORT_FN_VOID_COMPARE_WITH_CONTEXT_LAST_THEN_CONTEXT, {.void_compare_with_context_last_then_context = insertion_sort_v2}, .perf = PERF_SLOW},
    /* third-party sort functions */
    {"bentley_mcilroy_quicksort", SORT_FN_VOID_COMPARE_WITH_CONTEXT_LAST_THEN_CONTEXT, {.void_compare_with_context_last_then_context = bentley_mcilroy_quicksort}, .perf = PERF_FAST},
    {"ochs_smoothsort", SORT_FN_VOID_COMPARE_WITH_CONTEXT_LAST_THEN_CONTEXT, {.void_compare_with_context_last_then_context = ochs_smoothsort}, .perf = PERF_FAST},
    {"timsort", SORT_FN_INT_COMPARE_WITH_CONTEXT_LAST_THEN_CONTEXT, {.int_compare_with_context_last_then_context = timsort_r}, .perf = PERF_FAST},
    {"bsd_heapsort", SORT_FN_INT_COMPARE_WITH_CONTEXT_LAST_THEN_CONTEXT, {.int_compare_with_context_last_then_context = bsd_heapsort}, .perf = PERF_FAST},
    {"bsd_mergesort", SORT_FN_INT_COMPARE_WITH_CONTEXT_LAST_THEN_CONTEXT, {.int_compare_with_context_last_then_context = bsd_mergesort}, .perf = PERF_FAST},
};

static int compare_elem(const void *a_ptr, const void *b_ptr)
{
    elem_t a, b;
    memcpy(&a, a_ptr, sizeof(a));
    memcpy(&b, b_ptr, sizeof(b));
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
        case SORT_FN_VOID_NO_CONTEXT:
            sort->fn.void_no_context(base, nelems, size, compare_elem);
            break;
        case SORT_FN_INT_NO_CONTEXT:
            result = sort->fn.int_no_context(base, nelems, size, compare_elem);
            break;
        case SORT_FN_VOID_COMPARE_WITH_CONTEXT_FIRST_THEN_CONTEXT:
            sort->fn.void_compare_with_context_first_then_context(base, nelems, size, compare_elem_with_context, context);
            break;
        case SORT_FN_VOID_COMPARE_WITH_CONTEXT_LAST_THEN_CONTEXT:
            sort->fn.void_compare_with_context_last_then_context(base, nelems, size, compare_elem_with_context_last, context);
            break;
        case SORT_FN_INT_COMPARE_WITH_CONTEXT_LAST_THEN_CONTEXT:
            result = sort->fn.int_compare_with_context_last_then_context(base, nelems, size, compare_elem_with_context_last, context);
            break;
        case SORT_FN_VOID_CONTEXT_THEN_COMPARE_WITH_CONTEXT_FIRST:
            sort->fn.void_context_then_compare_with_context_first(base, nelems, size, context, compare_elem_with_context);
            break;
        case SORT_FN_VOID_CONTEXT_THEN_COMPARE_WITH_CONTEXT_LAST:
            sort->fn.void_context_then_compare_with_context_last(base, nelems, size, context, compare_elem_with_context_last);
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
        printf("Test '%s' failed for sort function %s!\n", test_name, sort->name);
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

static void array_init_ascending(char *array, elem_t array_size, size_t elem_size)
{
    char *elem = array;
    for (elem_t i = 0; i < array_size; i++) {
        memset(elem, 0, elem_size);
        memcpy(elem, &i, sizeof(elem_t));
        elem += elem_size;
    }
}

static void array_init_descending(char *array, elem_t array_size, size_t elem_size)
{
    char *elem = array;
    for (elem_t i = 0; i < array_size; i++) {
        elem_t value = array_size - 1 - i;
        memset(elem, 0, elem_size);
        memcpy(elem, &value, sizeof(elem_t));
        elem += elem_size;
    }
}

static void array_swap(char *array, size_t elem_size, size_t i, size_t j)
{
    char temp_buf[1024];
    char *temp = elem_size > sizeof(temp_buf) ? malloc(elem_size) : temp_buf;
    memcpy(temp, array + i * elem_size, elem_size);
    memcpy(array + i * elem_size, array + j * elem_size, elem_size);
    memcpy(array + j * elem_size, temp, elem_size);
    if (temp != temp_buf) {
        free(temp);
    }
}

static void array_random_shuffle(char *array, size_t array_size, size_t elem_size, uint32_t *seed)
{
    for (size_t i = 0; i < array_size; i++) {
        size_t j = random_uint32(seed) % (i + 1);
        array_swap(array, elem_size, i, j);
    }
}

static bool test_ascending_array(const sort_fn_t *sort, elem_t array_size, size_t elem_size, clock_t *out_time)
{
    char *array = calloc(array_size, elem_size);
    array_init_ascending(array, array_size, elem_size);
    bool result = test_sort(array, elem_size, array_size, sort, "ascending array", out_time);
    free(array);
    return result;
}

static bool test_mostly_ascending_array(const sort_fn_t *sort, elem_t array_size, size_t elem_size, random_seed_t *seed, clock_t *out_time)
{
    char *array = calloc(array_size, elem_size);
    array_init_ascending(array, array_size, elem_size);
    for (size_t i = 0; i < array_size / 10; i++) {
        size_t j = random_uint32(seed) % array_size;
        array_swap(array, elem_size, i, j);
    }
    bool result = test_sort(array, elem_size, array_size, sort, "mostly ascending array", out_time);
    free(array);
    return result;
}

static bool test_descending_array(const sort_fn_t *sort, elem_t array_size, size_t elem_size, clock_t *out_time)
{
    char *array = calloc(array_size, elem_size);
    array_init_descending(array, array_size, elem_size);
    bool result = test_sort(array, elem_size, array_size, sort, "descending array", out_time);
    free(array);
    return result;
}

static bool test_ascending_then_descending_array(const sort_fn_t *sort, elem_t array_size, size_t elem_size, clock_t *out_time)
{
    char *array = calloc(array_size, elem_size);
    elem_t middle = array_size / 2;
    array_init_ascending(array, middle, elem_size);
    array_init_descending(array + middle * elem_size, array_size - middle, elem_size);
    bool result = test_sort(array, elem_size, array_size, sort, "ascending then descending array", out_time);
    free(array);
    return result;
}

static bool test_sawtooth_array(const sort_fn_t *sort, elem_t array_size, size_t elem_size, clock_t *out_time)
{
    char *array = calloc(array_size, elem_size);
    elem_t segment_size = 10;
    for (elem_t i = 0; i < array_size; i += segment_size) {
        elem_t current_segment_size = (i + segment_size <= array_size) ? segment_size : (array_size - i);
        array_init_ascending(array + i * elem_size, current_segment_size, elem_size);
    }
    bool result = test_sort(array, elem_size, array_size, sort, "sawtooth array", out_time);
    free(array);
    return result;
}

static bool test_reverse_sawtooth_array(const sort_fn_t *sort, elem_t array_size, size_t elem_size, clock_t *out_time)
{
    char *array = calloc(array_size, elem_size);
    elem_t segment_size = 10;
    for (elem_t i = 0; i < array_size; i += segment_size) {
        elem_t current_segment_size = (i + segment_size <= array_size) ? segment_size : (array_size - i);
        array_init_descending(array + i * elem_size, current_segment_size, elem_size);
    }
    bool result = test_sort(array, elem_size, array_size, sort, "reverse sawtooth array", out_time);
    free(array);
    return result;
}

static bool test_random_array(const sort_fn_t *sort, elem_t array_size, size_t elem_size, random_seed_t *seed, clock_t *out_time)
{
    char *array = calloc(array_size, elem_size);
    array_init_ascending(array, array_size, elem_size);
    array_random_shuffle(array, array_size, elem_size, seed);
    bool result = test_sort(array, elem_size, array_size, sort, "random array", out_time);
    free(array);
    return result;
}

static bool run_tests(const sort_fn_t *sort, random_seed_t seed, elem_t array_size, size_t elem_size)
{
    printf("Testing sort function: %s\n", sort->name);

    clock_t total_time = 0;
    clock_t time = 0;

    if (!test_ascending_array(sort, array_size, elem_size, &time)) {
        return false;
    }
    total_time += time;
    if (!test_mostly_ascending_array(sort, array_size, elem_size, &seed, &time)) {
        return false;
    }
    total_time += time;
    if (!test_descending_array(sort, array_size, elem_size, &time)) {
        return false;
    }
    total_time += time;
    if (!test_ascending_then_descending_array(sort, array_size, elem_size, &time)) {
        return false;
    }
    total_time += time;
    if (!test_sawtooth_array(sort, array_size, elem_size, &time)) {
        return false;
    }
    total_time += time;
    if (!test_reverse_sawtooth_array(sort, array_size, elem_size, &time)) {
        return false;
    }
    total_time += time;
    if (!test_random_array(sort, array_size, elem_size, &seed, &time)) {
        return false;
    }
    total_time += time;

    // Don't print timing information in debug builds to avoid unfair comparisons
#ifdef NDEBUG
    double total_time_seconds = (double) total_time / CLOCKS_PER_SEC;
    if (total_time_seconds > 0.1) {
        printf("Time: %.2f seconds\n", total_time_seconds);
    } else if (total_time_seconds > 0.001) {
        printf("Time: %.2f milliseconds\n", total_time_seconds * 1000.0);
    } else {
        printf("Time: %.2f microseconds\n", total_time_seconds * 1000000.0);
    }
#endif

    return true;
}

static void usage(void)
{
    static const char *perf_names[] = {"\x1b[31mslow\x1b[0m", "\x1b[33m mid\x1b[0m", "\x1b[32mfast\x1b[0m"};
    printf("usage: test_sort [-f <function>] [-n <array-size>] [-s <elem-size>] [-r <seed>]\n");
    printf("available sort functions:\n");
    for (size_t i = 0; i < ARRAY_SIZE(sort_functions); i++) {
        printf("    %s  %s\n", perf_names[sort_functions[i].perf], sort_functions[i].name);
    }
}

int main(int argc, char **argv)
{
    const sort_fn_t *sort = NULL;
    elem_t array_size = 1000000;
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
            if (size <= 0 || size > ELEM_MAX) {
                fprintf(stderr, "error: invalid array size: %llu\n", size);
                usage();
                return 1;
            }
            array_size = (elem_t) size;
        } else if (strcmp(argv[i], "-s") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "error: missing argument to -s\n");
                usage();
                return 1;
            }
            unsigned long long size = strtoull(argv[++i], NULL, 10);
            if (size <= 0 || size < sizeof(elem_t) || size > 0x40000000) {
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

    printf("Array size: %u, Element size: %zu, Random seed: %u\n", array_size, elem_size, seed);
    if (!sort) {
        for (size_t i = 0; i < ARRAY_SIZE(sort_functions); i++) {
            if (sort_functions[i].perf > PERF_SLOW || array_size <= 10000) {
                if (!run_tests(&sort_functions[i], seed, array_size, elem_size)) {
                    return 1;
                }
            }
        }
    } else {
        if (!run_tests(sort, seed, array_size, elem_size)) {
            return 1;
        }
    }
    printf("All tests passed.\n");
    return 0;
}
