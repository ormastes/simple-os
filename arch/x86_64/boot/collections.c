/*
 * SimpleOS x86_64 BTreeMap / BTreeSet Runtime
 *
 * Sorted-array implementation for baremetal (freestanding, no libc).
 * Overrides the weak auto_stubs for __rt_btreemap_* and __rt_btreeset_*.
 *
 * Internal representation: SortedMap — a sorted parallel-array of keys
 * (and optionally values).  Linear scan for <=64 elements, binary search
 * above that.  Keys are ordered by raw int64_t comparison.
 *
 * Heap type tags:
 *   HEAP_BTREEMAP  = 8
 *   HEAP_BTREESET  = 9
 */

#include <stdint.h>
#include <stddef.h>
#include "../../common/baremetal_runtime.h"

typedef RuntimeValue RV;

#define HEAP_CLOSURE   5
#define HEAP_MODULE    6
#define HEAP_ENUM      7
#define HEAP_BTREEMAP  8
#define HEAP_BTREESET  9

/* Sorted parallel arrays of keys + values.
 * For BTreeSet, values == NULL. */
typedef struct {
    HeapHeader   hdr;     /* type = HEAP_BTREEMAP or HEAP_BTREESET */
    uint32_t     len;
    uint32_t     cap;
    RuntimeValue *keys;
    RuntimeValue *values; /* NULL for BTreeSet */
} SortedMap;

/* ===================================================================
 * External declarations — provided by baremetal_stubs.c / linker
 * =================================================================== */

extern void *malloc(size_t sz);
extern void  free(void *p);
/* rt_array_new / rt_array_push from baremetal_stubs.c */
extern RuntimeValue rt_array_new(RuntimeValue cap_val);
extern int8_t rt_array_push(RuntimeValue arr, RuntimeValue val);

/* ===================================================================
 * Internal helpers
 * =================================================================== */

/* Default and minimum initial capacity */
#define SORTEDMAP_INIT_CAP 16

/* Threshold: linear scan below this, binary search at or above */
#define BSEARCH_THRESHOLD 64

static void sm_memcpy_rv(RuntimeValue *dst, const RuntimeValue *src, uint32_t n)
{
    for (uint32_t i = 0; i < n; i++) dst[i] = src[i];
}

/* --- Comparison --------------------------------------------------- */

/* Compare two RuntimeValues for ordering.
 * Returns <0, 0, >0 (like strcmp).
 * We compare the raw int64_t bit-patterns so integers stay in numeric
 * order (tag bits are lowest 3, so two TAG_INT values compare correctly
 * after the shift inherent in the encoding). */
static inline int sm_cmp(RuntimeValue a, RuntimeValue b)
{
    /* Fast path: raw numeric comparison on the 64-bit values.
     * Works correctly for TAG_INT (values are shifted left 3, so
     * ordering is preserved).  For heap/float/special tags the
     * ordering is deterministic though not semantically meaningful
     * beyond providing a stable total order. */
    if (a < b) return -1;
    if (a > b) return  1;
    return 0;
}

/* --- Lookup: linear or binary search ------------------------------ */

/* Linear scan: return index of key, or -(insertion_point + 1) if not found.
 * Keys are sorted ascending. */
static int32_t sm_linear_find(const RuntimeValue *keys, uint32_t len, RuntimeValue key)
{
    for (uint32_t i = 0; i < len; i++) {
        int c = sm_cmp(keys[i], key);
        if (c == 0) return (int32_t)i;
        if (c > 0)  return -(int32_t)(i + 1);  /* keys[i] > key: insert here */
    }
    return -(int32_t)(len + 1);  /* append at end */
}

/* Binary search: return index of key, or -(insertion_point + 1). */
static int32_t sm_binary_find(const RuntimeValue *keys, uint32_t len, RuntimeValue key)
{
    uint32_t lo = 0, hi = len;
    while (lo < hi) {
        uint32_t mid = lo + (hi - lo) / 2;
        int c = sm_cmp(keys[mid], key);
        if (c == 0) return (int32_t)mid;
        if (c < 0)  lo = mid + 1;
        else        hi = mid;
    }
    return -(int32_t)(lo + 1);
}

/* Unified find: picks strategy based on len. */
static int32_t sm_find(const RuntimeValue *keys, uint32_t len, RuntimeValue key)
{
    if (len < BSEARCH_THRESHOLD)
        return sm_linear_find(keys, len, key);
    return sm_binary_find(keys, len, key);
}

/* Convert the negative encoding back to an insertion index. */
static inline uint32_t sm_insert_index(int32_t result)
{
    return (uint32_t)(-(result + 1));
}

/* --- Decode helpers ----------------------------------------------- */

static inline SortedMap *decode_btreemap(RuntimeValue v)
{
    if (!IS_HEAP(v)) return (SortedMap *)0;
    SortedMap *m = (SortedMap *)DECODE_PTR(v);
    if (!m || m->hdr.type != HEAP_BTREEMAP) return (SortedMap *)0;
    return m;
}

static inline SortedMap *decode_btreeset(RuntimeValue v)
{
    if (!IS_HEAP(v)) return (SortedMap *)0;
    SortedMap *m = (SortedMap *)DECODE_PTR(v);
    if (!m || m->hdr.type != HEAP_BTREESET) return (SortedMap *)0;
    return m;
}

/* --- Grow --------------------------------------------------------- */

static void sm_grow(SortedMap *m)
{
    uint32_t new_cap = m->cap * 2;
    if (new_cap < SORTEDMAP_INIT_CAP) new_cap = SORTEDMAP_INIT_CAP;

    RuntimeValue *nk = (RuntimeValue *)malloc(new_cap * sizeof(RuntimeValue));
    if (!nk) return; /* OOM: silently fail on bare metal */
    sm_memcpy_rv(nk, m->keys, m->len);
    /* Old keys leak (bump allocator, free is no-op) */
    m->keys = nk;

    if (m->values) {
        RuntimeValue *nv = (RuntimeValue *)malloc(new_cap * sizeof(RuntimeValue));
        if (!nv) return;
        sm_memcpy_rv(nv, m->values, m->len);
        m->values = nv;
    }

    m->cap = new_cap;
}

/* --- Shift elements for insert / remove --------------------------- */

static void sm_shift_right(RuntimeValue *arr, uint32_t from, uint32_t len)
{
    /* Shift elements [from .. len-1] one position to the right. */
    for (uint32_t i = len; i > from; i--) {
        arr[i] = arr[i - 1];
    }
}

static void sm_shift_left(RuntimeValue *arr, uint32_t from, uint32_t len)
{
    /* Shift elements [from+1 .. len-1] one position to the left. */
    for (uint32_t i = from; i + 1 < len; i++) {
        arr[i] = arr[i + 1];
    }
}

/* --- Create a RuntimeArray with given elements -------------------- */

static RuntimeValue sm_make_array(const RuntimeValue *items, uint32_t count)
{
    /* rt_array_new takes raw capacity (see baremetal_stubs.c comment). */
    int64_t raw_cap = (int64_t)(count > 0 ? count : 1);
    RuntimeValue arr = rt_array_new((RuntimeValue)raw_cap);
    if (IS_NIL(arr)) return NIL_VALUE;
    for (uint32_t i = 0; i < count; i++) {
        rt_array_push(arr, items[i]);
    }
    return arr;
}

/* ===================================================================
 * BTreeMap — sorted array of key-value pairs
 * =================================================================== */

RuntimeValue __rt_btreemap_new(void)
{
    SortedMap *m = (SortedMap *)malloc(sizeof(SortedMap));
    if (!m) return NIL_VALUE;
    m->hdr.type = HEAP_BTREEMAP;
    m->hdr.size = (uint32_t)sizeof(SortedMap);
    m->len = 0;
    m->cap = SORTEDMAP_INIT_CAP;
    m->keys   = (RuntimeValue *)malloc(SORTEDMAP_INIT_CAP * sizeof(RuntimeValue));
    m->values = (RuntimeValue *)malloc(SORTEDMAP_INIT_CAP * sizeof(RuntimeValue));
    if (!m->keys || !m->values) return NIL_VALUE;
    return ENCODE_PTR(m);
}

RuntimeValue __rt_btreemap_insert(RV map, RV key, RV value)
{
    SortedMap *m = decode_btreemap(map);
    if (!m) return NIL_VALUE;

    int32_t pos = sm_find(m->keys, m->len, key);
    if (pos >= 0) {
        /* Key exists — update value in place */
        m->values[pos] = value;
        return map;
    }

    /* Not found — insert at the sorted position */
    uint32_t idx = sm_insert_index(pos);
    if (m->len >= m->cap) sm_grow(m);
    if (m->len >= m->cap) return map; /* grow failed */

    sm_shift_right(m->keys,   idx, m->len);
    sm_shift_right(m->values, idx, m->len);
    m->keys[idx]   = key;
    m->values[idx] = value;
    m->len++;
    return map;
}

RuntimeValue __rt_btreemap_get(RV map, RV key)
{
    SortedMap *m = decode_btreemap(map);
    if (!m) return NIL_VALUE;

    int32_t pos = sm_find(m->keys, m->len, key);
    if (pos >= 0) return m->values[pos];
    return NIL_VALUE;
}

RuntimeValue __rt_btreemap_remove(RV map, RV key)
{
    SortedMap *m = decode_btreemap(map);
    if (!m) return map;

    int32_t pos = sm_find(m->keys, m->len, key);
    if (pos < 0) return map; /* not found */

    sm_shift_left(m->keys,   (uint32_t)pos, m->len);
    sm_shift_left(m->values, (uint32_t)pos, m->len);
    m->len--;
    m->keys[m->len]   = NIL_VALUE;
    m->values[m->len]  = NIL_VALUE;
    return map;
}

RuntimeValue __rt_btreemap_contains_key(RV map, RV key)
{
    SortedMap *m = decode_btreemap(map);
    if (!m) return ENCODE_INT(0);
    int32_t pos = sm_find(m->keys, m->len, key);
    return ENCODE_INT(pos >= 0 ? 1 : 0);
}

RuntimeValue __rt_btreemap_len(RV map)
{
    SortedMap *m = decode_btreemap(map);
    if (!m) return ENCODE_INT(0);
    return ENCODE_INT((int64_t)m->len);
}

RuntimeValue __rt_btreemap_clear(RV map)
{
    SortedMap *m = decode_btreemap(map);
    if (!m) return map;
    m->len = 0;
    return map;
}

RuntimeValue __rt_btreemap_keys(RV map)
{
    SortedMap *m = decode_btreemap(map);
    if (!m) return NIL_VALUE;
    return sm_make_array(m->keys, m->len);
}

RuntimeValue __rt_btreemap_values(RV map)
{
    SortedMap *m = decode_btreemap(map);
    if (!m) return NIL_VALUE;
    return sm_make_array(m->values, m->len);
}

RuntimeValue __rt_btreemap_entries(RV map)
{
    SortedMap *m = decode_btreemap(map);
    if (!m) return NIL_VALUE;

    int64_t raw_cap = (int64_t)(m->len > 0 ? m->len : 1);
    RuntimeValue outer = rt_array_new((RuntimeValue)raw_cap);
    if (IS_NIL(outer)) return NIL_VALUE;

    for (uint32_t i = 0; i < m->len; i++) {
        /* Each entry is a 2-element array [key, value] */
        RuntimeValue pair = rt_array_new((RuntimeValue)2);
        if (IS_NIL(pair)) continue;
        rt_array_push(pair, m->keys[i]);
        rt_array_push(pair, m->values[i]);
        rt_array_push(outer, pair);
    }
    return outer;
}

RuntimeValue __rt_btreemap_first_key(RV map)
{
    SortedMap *m = decode_btreemap(map);
    if (!m || m->len == 0) return NIL_VALUE;
    return m->keys[0];
}

RuntimeValue __rt_btreemap_last_key(RV map)
{
    SortedMap *m = decode_btreemap(map);
    if (!m || m->len == 0) return NIL_VALUE;
    return m->keys[m->len - 1];
}

/* Drop (no-op on bump allocator, but clears the map) */
RuntimeValue __rt_btreemap_drop(RV map)
{
    SortedMap *m = decode_btreemap(map);
    if (!m) return ENCODE_INT(0);
    m->len = 0;
    return ENCODE_INT(1);
}

/* ===================================================================
 * BTreeSet — sorted array (values pointer is NULL)
 * =================================================================== */

RuntimeValue __rt_btreeset_new(void)
{
    SortedMap *m = (SortedMap *)malloc(sizeof(SortedMap));
    if (!m) return NIL_VALUE;
    m->hdr.type = HEAP_BTREESET;
    m->hdr.size = (uint32_t)sizeof(SortedMap);
    m->len = 0;
    m->cap = SORTEDMAP_INIT_CAP;
    m->keys   = (RuntimeValue *)malloc(SORTEDMAP_INIT_CAP * sizeof(RuntimeValue));
    m->values = (RuntimeValue *)0;  /* BTreeSet: no values array */
    if (!m->keys) return NIL_VALUE;
    return ENCODE_PTR(m);
}

RuntimeValue __rt_btreeset_insert(RV set, RV value)
{
    SortedMap *m = decode_btreeset(set);
    if (!m) return set;

    int32_t pos = sm_find(m->keys, m->len, value);
    if (pos >= 0) return set; /* already present */

    uint32_t idx = sm_insert_index(pos);
    if (m->len >= m->cap) sm_grow(m);
    if (m->len >= m->cap) return set; /* grow failed */

    sm_shift_right(m->keys, idx, m->len);
    m->keys[idx] = value;
    m->len++;
    return set;
}

RuntimeValue __rt_btreeset_remove(RV set, RV value)
{
    SortedMap *m = decode_btreeset(set);
    if (!m) return set;

    int32_t pos = sm_find(m->keys, m->len, value);
    if (pos < 0) return set;

    sm_shift_left(m->keys, (uint32_t)pos, m->len);
    m->len--;
    m->keys[m->len] = NIL_VALUE;
    return set;
}

RuntimeValue __rt_btreeset_contains(RV set, RV value)
{
    SortedMap *m = decode_btreeset(set);
    if (!m) return ENCODE_INT(0);
    int32_t pos = sm_find(m->keys, m->len, value);
    return ENCODE_INT(pos >= 0 ? 1 : 0);
}

RuntimeValue __rt_btreeset_len(RV set)
{
    SortedMap *m = decode_btreeset(set);
    if (!m) return ENCODE_INT(0);
    return ENCODE_INT((int64_t)m->len);
}

RuntimeValue __rt_btreeset_clear(RV set)
{
    SortedMap *m = decode_btreeset(set);
    if (!m) return set;
    m->len = 0;
    return set;
}

RuntimeValue __rt_btreeset_first(RV set)
{
    SortedMap *m = decode_btreeset(set);
    if (!m || m->len == 0) return NIL_VALUE;
    return m->keys[0];
}

RuntimeValue __rt_btreeset_last(RV set)
{
    SortedMap *m = decode_btreeset(set);
    if (!m || m->len == 0) return NIL_VALUE;
    return m->keys[m->len - 1];
}

RuntimeValue __rt_btreeset_to_array(RV set)
{
    SortedMap *m = decode_btreeset(set);
    if (!m) return NIL_VALUE;
    return sm_make_array(m->keys, m->len);
}

/* --- Set algebra operations --------------------------------------- */

/* is_subset: every element of a is in b */
RuntimeValue __rt_btreeset_is_subset(RV a, RV b)
{
    SortedMap *sa = decode_btreeset(a);
    SortedMap *sb = decode_btreeset(b);
    if (!sa || !sb) return ENCODE_INT(0);

    /* Both arrays are sorted — merge-walk */
    uint32_t ia = 0, ib = 0;
    while (ia < sa->len && ib < sb->len) {
        int c = sm_cmp(sa->keys[ia], sb->keys[ib]);
        if (c == 0) { ia++; ib++; }
        else if (c > 0) { ib++; }  /* sb element not in sa: skip */
        else { return ENCODE_INT(0); } /* sa element not in sb */
    }
    /* If we exhausted sa, it is a subset */
    return ENCODE_INT(ia >= sa->len ? 1 : 0);
}

/* is_superset: every element of b is in a */
RuntimeValue __rt_btreeset_is_superset(RV a, RV b)
{
    return __rt_btreeset_is_subset(b, a);
}

/* difference: elements in a but not in b — returns new set */
RuntimeValue __rt_btreeset_difference(RV a, RV b)
{
    SortedMap *sa = decode_btreeset(a);
    SortedMap *sb = decode_btreeset(b);
    if (!sa) return NIL_VALUE;

    RuntimeValue result = __rt_btreeset_new();
    SortedMap *r = decode_btreeset(result);
    if (!r) return NIL_VALUE;

    if (!sb) {
        /* b is invalid/nil — result is a copy of a */
        for (uint32_t i = 0; i < sa->len; i++) {
            result = __rt_btreeset_insert(result, sa->keys[i]);
        }
        return result;
    }

    /* Merge-walk: emit elements of a that are not in b */
    uint32_t ia = 0, ib = 0;
    while (ia < sa->len && ib < sb->len) {
        int c = sm_cmp(sa->keys[ia], sb->keys[ib]);
        if (c < 0) {
            result = __rt_btreeset_insert(result, sa->keys[ia]);
            ia++;
        } else if (c > 0) {
            ib++;
        } else {
            ia++; ib++; /* present in both — skip */
        }
    }
    /* Remaining elements of a */
    while (ia < sa->len) {
        result = __rt_btreeset_insert(result, sa->keys[ia]);
        ia++;
    }
    return result;
}

/* intersection: elements in both a and b — returns new set */
RuntimeValue __rt_btreeset_intersection(RV a, RV b)
{
    SortedMap *sa = decode_btreeset(a);
    SortedMap *sb = decode_btreeset(b);
    if (!sa || !sb) return __rt_btreeset_new();

    RuntimeValue result = __rt_btreeset_new();

    /* Merge-walk: emit elements present in both */
    uint32_t ia = 0, ib = 0;
    while (ia < sa->len && ib < sb->len) {
        int c = sm_cmp(sa->keys[ia], sb->keys[ib]);
        if (c < 0) { ia++; }
        else if (c > 0) { ib++; }
        else {
            result = __rt_btreeset_insert(result, sa->keys[ia]);
            ia++; ib++;
        }
    }
    return result;
}

/* symmetric_difference: elements in either but not both — returns new set */
RuntimeValue __rt_btreeset_symmetric_difference(RV a, RV b)
{
    SortedMap *sa = decode_btreeset(a);
    SortedMap *sb = decode_btreeset(b);

    RuntimeValue result = __rt_btreeset_new();

    if (!sa && !sb) return result;
    if (!sa) {
        /* Copy b */
        for (uint32_t i = 0; i < sb->len; i++)
            result = __rt_btreeset_insert(result, sb->keys[i]);
        return result;
    }
    if (!sb) {
        /* Copy a */
        for (uint32_t i = 0; i < sa->len; i++)
            result = __rt_btreeset_insert(result, sa->keys[i]);
        return result;
    }

    /* Merge-walk: emit elements present in exactly one */
    uint32_t ia = 0, ib = 0;
    while (ia < sa->len && ib < sb->len) {
        int c = sm_cmp(sa->keys[ia], sb->keys[ib]);
        if (c < 0) {
            result = __rt_btreeset_insert(result, sa->keys[ia]);
            ia++;
        } else if (c > 0) {
            result = __rt_btreeset_insert(result, sb->keys[ib]);
            ib++;
        } else {
            ia++; ib++; /* skip common elements */
        }
    }
    while (ia < sa->len) {
        result = __rt_btreeset_insert(result, sa->keys[ia]);
        ia++;
    }
    while (ib < sb->len) {
        result = __rt_btreeset_insert(result, sb->keys[ib]);
        ib++;
    }
    return result;
}

/* union: all elements from both a and b — returns new set */
RuntimeValue __rt_btreeset_union(RV a, RV b)
{
    SortedMap *sa = decode_btreeset(a);
    SortedMap *sb = decode_btreeset(b);

    RuntimeValue result = __rt_btreeset_new();

    if (!sa && !sb) return result;
    if (!sa) {
        for (uint32_t i = 0; i < sb->len; i++)
            result = __rt_btreeset_insert(result, sb->keys[i]);
        return result;
    }
    if (!sb) {
        for (uint32_t i = 0; i < sa->len; i++)
            result = __rt_btreeset_insert(result, sa->keys[i]);
        return result;
    }

    /* Merge-walk: emit all elements, de-duplicated */
    uint32_t ia = 0, ib = 0;
    while (ia < sa->len && ib < sb->len) {
        int c = sm_cmp(sa->keys[ia], sb->keys[ib]);
        if (c < 0) {
            result = __rt_btreeset_insert(result, sa->keys[ia]);
            ia++;
        } else if (c > 0) {
            result = __rt_btreeset_insert(result, sb->keys[ib]);
            ib++;
        } else {
            result = __rt_btreeset_insert(result, sa->keys[ia]);
            ia++; ib++;
        }
    }
    while (ia < sa->len) {
        result = __rt_btreeset_insert(result, sa->keys[ia]);
        ia++;
    }
    while (ib < sb->len) {
        result = __rt_btreeset_insert(result, sb->keys[ib]);
        ib++;
    }
    return result;
}

/* Drop (no-op on bump allocator, but clears the set) */
RuntimeValue __rt_btreeset_drop(RV set)
{
    SortedMap *m = decode_btreeset(set);
    if (!m) return ENCODE_INT(0);
    m->len = 0;
    return ENCODE_INT(1);
}
