/*
 * From "Engineering a Sort Function" by Jon L. Bentley and M. Douglas McIlroy (November 1993)
 * Software: Practice and Experience, Volume 23, Issue 11, 1249–1265
 * 
 * With some minor modifications and cleanup.
 */

#include <stddef.h>
#include <stdint.h>

typedef size_t WORD;

#define W sizeof(WORD) /* must be a power of 2 */
#define exch(a, b, t) do { t = a; a = b; b = t; } while (0)
#define min(a, b) ((a) < (b) ? (a) : (b))

static char *med3(char *a, char *b, char *c, int (*cmp)(const void *, const void *, void *), void *ctx)
{
    return cmp(a, b, ctx) < 0 ? (cmp(b, c, ctx) < 0 ? b : cmp(a, c, ctx) < 0 ? c : a)
                              : (cmp(b, c, ctx) > 0 ? b : cmp(a, c, ctx) > 0 ? c : a);
}

static void swapfunc(char *a, char *b, size_t n, int swaptype)
{
    if (swaptype <= 1) {
        for (WORD t; n > 0; a += W, b += W, n -= W) {
            exch(*(WORD*)a, *(WORD*)b, t);
        }
    } else {
        for (char t; n > 0; a += 1, b += 1, n -= 1) {
            exch(*a, *b, t);
        }
    }
}

#define swap(a, b) do { if (swaptype != 0) { swapfunc(a, b, es, swaptype); } else { exch(*(WORD*)(a), *(WORD*)(b), t); } } while (0)
#define vecswap(a, b, n) do { if (n > 0) swapfunc(a, b, n, swaptype); } while (0)

void bentley_mcilroy_quicksort(char *a, size_t n, size_t es, int (*cmp)(const void *, const void *, void *), void *ctx)
{
    char *pa, *pb, *pc, *pd, *pl, *pm, *pn, *pv;
    int r;
    WORD t, v;
    size_t s;
    const int swaptype = ((uintptr_t)a | es) % W ? 2 : es > W ? 1 : 0;
    if (n < 7) { /* Insertion sort on smallest arrays */
        for (pm = a + es; pm < a + n * es; pm += es) {
            for (pl = pm; pl > a && cmp(pl - es, pl, ctx) > 0; pl -= es) {
                swap(pl, pl - es);
            }
        }
        return;
    }
    pm = a + (n / 2) * es; /* Small arrays, middle element */
    if (n > 7) {
        pl = a;
        pn = a + (n - 1) * es;
        if (n > 40) { /* Big arrays, pseudomedian of 9 */
            s = (n / 8) * es;
            pl = med3(pl, pl + s, pl + 2 * s, cmp, ctx);
            pm = med3(pm - s, pm, pm + s, cmp, ctx);
            pn = med3(pn - 2 * s, pn - s, pn, cmp, ctx);
        }
        pm = med3(pl, pm, pn, cmp, ctx); /* Mid-size, med of 3 */
    }
    /* pv points to partition value */
    if (swaptype != 0) {
        pv = a;
        swap(pv, pm);
    } else {
        pv = (char*)&v;
        v = *(WORD*)pm;
    }
    pa = pb = a;
    pc = pd = a + (n - 1) * es;
    for (;;) {
        while (pb <= pc && (r = cmp(pb, pv, ctx)) <= 0) {
            if (r == 0) {
                swap(pa, pb);
                pa += es;
            }
            pb += es;
        }
        while (pc >= pb && (r = cmp(pc, pv, ctx)) >= 0) {
            if (r == 0) {
                swap(pc, pd);
                pd -= es;
            }
            pc -= es;
        }
        if (pb > pc) break;
        swap(pb, pc);
        pb += es;
        pc -= es;
    }
    pn = a + n * es;
    s = (size_t) min(pa - a, pb - pa);
    vecswap(a, pb - s, s);
    s = min((size_t)(pd - pc), (size_t)(pn - pd) - es);
    vecswap(pb, pn - s, s);
    if ((s = (size_t)(pb - pa)) > es) bentley_mcilroy_quicksort(a, s / es, es, cmp, ctx);
    if ((s = (size_t)(pd - pc)) > es) bentley_mcilroy_quicksort(pn - s, s / es, es, cmp, ctx);
}
