/*
 * SimpleOS x86_64 Baremetal HashMap / HashSet Runtime
 *
 * Implements __rt_hashmap_* and __rt_hashset_* functions for the Simple
 * language compiler targeting freestanding x86_64.
 *
 * Data structure: open addressing with linear probing.
 * Hash function: splitmix64 finalizer on raw RuntimeValue bits.
 * Load factor threshold: 75% (grow by doubling, never shrink).
 *
 * Depends on:
 *   - malloc()          (bump allocator, free is no-op)
 *   - rt_native_eq()    (RuntimeValue equality, returns raw 1/0)
 *   - rt_array_new()    (create RuntimeArray)
 *   - rt_array_push()   (append to RuntimeArray)
 *
 * The auto_stubs.c weak symbols are overridden by these definitions.
 */

#include <stdint.h>
#include <stddef.h>
#include "../../common/baremetal_runtime.h"

/* ===================================================================
 * HashMap / HashSet internal structure
 *
 * A single struct serves both HashMap and HashSet.  For HashSet the
 * `values` pointer is NULL — only the `keys` array is used.
 * =================================================================== */

#define HEAP_HASHMAP 9
#define HEAP_HASHSET 10

#define HASH_INITIAL_CAP 16

/* Sentinel values stored in the keys[] array to mark slot state.
 * These are chosen so they can never collide with a valid RuntimeValue:
 *   - Valid ints have tag 0b000 in the low 3 bits.
 *   - Valid heap ptrs have tag 0b001.
 *   - Valid floats have tag 0b010.
 *   - Valid specials have tag 0b011 (only NIL = 0x3 is used).
 * The sentinels use 0x7FFFFFFFFFFFFFFF / 0x7FFFFFFFFFFFFFFE which have
 * tag bits 0b111 — never produced by ENCODE_INT/ENCODE_PTR/ENCODE_FLOAT. */
#define HASH_EMPTY     ((RuntimeValue)0x7FFFFFFFFFFFFFFFLL)
#define HASH_TOMBSTONE ((RuntimeValue)0x7FFFFFFFFFFFFFFELL)

typedef struct {
    HeapHeader    hdr;       /* type = HEAP_HASHMAP or HEAP_HASHSET */
    uint32_t      len;       /* number of live entries */
    uint32_t      cap;       /* total slots (always a power of 2) */
    RuntimeValue *keys;
    RuntimeValue *values;    /* NULL for HashSet */
} HashMap;

/* ===================================================================
 * External dependencies
 * =================================================================== */

extern void *malloc(size_t sz);
extern RuntimeValue rt_native_eq(RuntimeValue a, RuntimeValue b);
extern RuntimeValue rt_array_new(RuntimeValue cap_val);
extern int8_t rt_array_push(RuntimeValue arr, RuntimeValue val);

/* ===================================================================
 * Hash function — splitmix64 finalizer
 * =================================================================== */

static uint64_t rv_hash(RuntimeValue v)
{
    uint64_t h = (uint64_t)v;
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= h >> 33;
    return h;
}

/* ===================================================================
 * Internal helpers
 * =================================================================== */

static inline int slot_is_empty(RuntimeValue v)
{
    return v == HASH_EMPTY;
}

static inline int slot_is_tombstone(RuntimeValue v)
{
    return v == HASH_TOMBSTONE;
}

static inline int slot_is_vacant(RuntimeValue v)
{
    return v == HASH_EMPTY || v == HASH_TOMBSTONE;
}

/* Allocate a keys (and optionally values) array of `cap` slots,
 * initialised to HASH_EMPTY. */
static RuntimeValue *alloc_slots(uint32_t cap)
{
    RuntimeValue *s = (RuntimeValue *)malloc((size_t)cap * sizeof(RuntimeValue));
    if (!s) return (RuntimeValue *)0;
    for (uint32_t i = 0; i < cap; i++)
        s[i] = HASH_EMPTY;
    return s;
}

/* Decode a RuntimeValue to a HashMap*, validating the heap tag and type. */
static HashMap *decode_hashmap(RuntimeValue v)
{
    if (!IS_HEAP(v)) return (HashMap *)0;
    HashMap *m = (HashMap *)DECODE_PTR(v);
    if (!m) return (HashMap *)0;
    if (m->hdr.type != HEAP_HASHMAP && m->hdr.type != HEAP_HASHSET)
        return (HashMap *)0;
    return m;
}

/* Find the slot index for `key`.  Returns the index where key lives,
 * or the first vacant slot if key is not present.
 * `found` is set to 1 if the key was found, 0 otherwise. */
static uint32_t probe_find(HashMap *m, RuntimeValue key, int *found)
{
    uint32_t mask = m->cap - 1;
    uint32_t idx  = (uint32_t)(rv_hash(key) & mask);
    uint32_t first_tombstone = m->cap; /* sentinel: not found yet */

    for (uint32_t i = 0; i < m->cap; i++) {
        RuntimeValue k = m->keys[idx];
        if (slot_is_empty(k)) {
            /* Hit an empty slot — key not in table.
             * Return the first tombstone we passed (for insertion reuse)
             * or this empty slot. */
            *found = 0;
            return (first_tombstone < m->cap) ? first_tombstone : idx;
        }
        if (slot_is_tombstone(k)) {
            if (first_tombstone >= m->cap)
                first_tombstone = idx;
        } else if (rt_native_eq(k, key)) {
            *found = 1;
            return idx;
        }
        idx = (idx + 1) & mask;
    }
    /* Table full (should not happen if load factor is maintained) */
    *found = 0;
    return (first_tombstone < m->cap) ? first_tombstone : 0;
}

/* Grow the table to double capacity and rehash all live entries. */
static void hashmap_grow(HashMap *m)
{
    uint32_t old_cap = m->cap;
    uint32_t new_cap = old_cap * 2;
    if (new_cap < HASH_INITIAL_CAP) new_cap = HASH_INITIAL_CAP;

    RuntimeValue *new_keys = alloc_slots(new_cap);
    if (!new_keys) return; /* OOM: silently fail on bare metal */

    RuntimeValue *new_values = (RuntimeValue *)0;
    if (m->values) {
        new_values = alloc_slots(new_cap);
        if (!new_values) return; /* OOM */
    }

    RuntimeValue *old_keys   = m->keys;
    RuntimeValue *old_values = m->values;
    uint32_t mask = new_cap - 1;

    /* Rehash all live entries */
    for (uint32_t i = 0; i < old_cap; i++) {
        RuntimeValue k = old_keys[i];
        if (slot_is_vacant(k)) continue;

        uint32_t idx = (uint32_t)(rv_hash(k) & mask);
        while (!slot_is_empty(new_keys[idx]))
            idx = (idx + 1) & mask;

        new_keys[idx] = k;
        if (new_values && old_values)
            new_values[idx] = old_values[i];
    }

    /* Bump allocator: old arrays leak — acceptable on bare metal */
    m->keys   = new_keys;
    m->values = new_values;
    m->cap    = new_cap;
}

/* Check load factor and grow if necessary (>= 75% full). */
static inline void maybe_grow(HashMap *m)
{
    /* Threshold: len * 4 >= cap * 3  <==>  len/cap >= 0.75 */
    if ((uint64_t)m->len * 4 >= (uint64_t)m->cap * 3)
        hashmap_grow(m);
}

/* ===================================================================
 * HashMap API
 * =================================================================== */

RuntimeValue __rt_hashmap_new(void)
{
    HashMap *m = (HashMap *)malloc(sizeof(HashMap));
    if (!m) return NIL_VALUE;

    m->hdr.type = HEAP_HASHMAP;
    m->hdr.size = (uint32_t)sizeof(HashMap);
    m->len      = 0;
    m->cap      = HASH_INITIAL_CAP;
    m->keys     = alloc_slots(HASH_INITIAL_CAP);
    m->values   = alloc_slots(HASH_INITIAL_CAP);

    if (!m->keys || !m->values) return NIL_VALUE;
    return ENCODE_PTR(m);
}

RuntimeValue __rt_hashmap_insert(RuntimeValue map, RuntimeValue key, RuntimeValue value)
{
    HashMap *m = decode_hashmap(map);
    if (!m || !m->values) return NIL_VALUE;

    maybe_grow(m);

    int found = 0;
    uint32_t idx = probe_find(m, key, &found);
    if (found) {
        /* Key already present — update value, return old value */
        RuntimeValue old = m->values[idx];
        m->values[idx] = value;
        return old;
    }

    /* Insert into vacant slot */
    m->keys[idx]   = key;
    m->values[idx] = value;
    m->len++;
    return NIL_VALUE;
}

RuntimeValue __rt_hashmap_get(RuntimeValue map, RuntimeValue key)
{
    HashMap *m = decode_hashmap(map);
    if (!m || !m->values) return NIL_VALUE;

    int found = 0;
    uint32_t idx = probe_find(m, key, &found);
    if (found)
        return m->values[idx];
    return NIL_VALUE;
}

RuntimeValue __rt_hashmap_remove(RuntimeValue map, RuntimeValue key)
{
    HashMap *m = decode_hashmap(map);
    if (!m || !m->values) return NIL_VALUE;

    int found = 0;
    uint32_t idx = probe_find(m, key, &found);
    if (!found) return NIL_VALUE;

    RuntimeValue removed = m->values[idx];
    m->keys[idx]   = HASH_TOMBSTONE;
    m->values[idx] = HASH_EMPTY;
    m->len--;
    return removed;
}

RuntimeValue __rt_hashmap_contains_key(RuntimeValue map, RuntimeValue key)
{
    HashMap *m = decode_hashmap(map);
    if (!m || !m->values) return FALSE_VALUE;

    int found = 0;
    probe_find(m, key, &found);
    return found ? TRUE_VALUE : FALSE_VALUE;
}

RuntimeValue __rt_hashmap_len(RuntimeValue map)
{
    HashMap *m = decode_hashmap(map);
    if (!m) return ENCODE_INT(0);
    return ENCODE_INT(m->len);
}

RuntimeValue __rt_hashmap_keys(RuntimeValue map)
{
    HashMap *m = decode_hashmap(map);
    if (!m) return NIL_VALUE;

    RuntimeValue arr = rt_array_new(ENCODE_INT(m->len > 0 ? m->len : 1));
    for (uint32_t i = 0; i < m->cap; i++) {
        if (!slot_is_vacant(m->keys[i]))
            rt_array_push(arr, m->keys[i]);
    }
    return arr;
}

RuntimeValue __rt_hashmap_values(RuntimeValue map)
{
    HashMap *m = decode_hashmap(map);
    if (!m || !m->values) return NIL_VALUE;

    RuntimeValue arr = rt_array_new(ENCODE_INT(m->len > 0 ? m->len : 1));
    for (uint32_t i = 0; i < m->cap; i++) {
        if (!slot_is_vacant(m->keys[i]))
            rt_array_push(arr, m->values[i]);
    }
    return arr;
}

RuntimeValue __rt_hashmap_entries(RuntimeValue map)
{
    HashMap *m = decode_hashmap(map);
    if (!m || !m->values) return NIL_VALUE;

    RuntimeValue arr = rt_array_new(ENCODE_INT(m->len > 0 ? m->len : 1));
    for (uint32_t i = 0; i < m->cap; i++) {
        if (!slot_is_vacant(m->keys[i])) {
            RuntimeValue pair = rt_array_new(ENCODE_INT(2));
            rt_array_push(pair, m->keys[i]);
            rt_array_push(pair, m->values[i]);
            rt_array_push(arr, pair);
        }
    }
    return arr;
}

/* ===================================================================
 * HashSet API
 *
 * Reuses the same HashMap struct with values == NULL.
 * =================================================================== */

RuntimeValue __rt_hashset_new(void)
{
    HashMap *m = (HashMap *)malloc(sizeof(HashMap));
    if (!m) return NIL_VALUE;

    m->hdr.type = HEAP_HASHSET;
    m->hdr.size = (uint32_t)sizeof(HashMap);
    m->len      = 0;
    m->cap      = HASH_INITIAL_CAP;
    m->keys     = alloc_slots(HASH_INITIAL_CAP);
    m->values   = (RuntimeValue *)0;  /* NULL — distinguishes set from map */

    if (!m->keys) return NIL_VALUE;
    return ENCODE_PTR(m);
}

RuntimeValue __rt_hashset_insert(RuntimeValue set, RuntimeValue value)
{
    HashMap *m = decode_hashmap(set);
    if (!m) return NIL_VALUE;

    maybe_grow(m);

    int found = 0;
    uint32_t idx = probe_find(m, value, &found);
    if (found)
        return FALSE_VALUE; /* already present */

    m->keys[idx] = value;
    m->len++;
    return TRUE_VALUE; /* newly inserted */
}

RuntimeValue __rt_hashset_remove(RuntimeValue set, RuntimeValue value)
{
    HashMap *m = decode_hashmap(set);
    if (!m) return FALSE_VALUE;

    int found = 0;
    uint32_t idx = probe_find(m, value, &found);
    if (!found) return FALSE_VALUE;

    m->keys[idx] = HASH_TOMBSTONE;
    m->len--;
    return TRUE_VALUE;
}

RuntimeValue __rt_hashset_contains(RuntimeValue set, RuntimeValue value)
{
    HashMap *m = decode_hashmap(set);
    if (!m) return FALSE_VALUE;

    int found = 0;
    probe_find(m, value, &found);
    return found ? TRUE_VALUE : FALSE_VALUE;
}

RuntimeValue __rt_hashset_len(RuntimeValue set)
{
    HashMap *m = decode_hashmap(set);
    if (!m) return ENCODE_INT(0);
    return ENCODE_INT(m->len);
}

RuntimeValue __rt_hashset_drop(RuntimeValue set)
{
    /* No-op on bump allocator — memory is never freed. */
    (void)set;
    return NIL_VALUE;
}

RuntimeValue __rt_hashset_to_array(RuntimeValue set)
{
    HashMap *m = decode_hashmap(set);
    if (!m) return NIL_VALUE;

    RuntimeValue arr = rt_array_new(ENCODE_INT(m->len > 0 ? m->len : 1));
    for (uint32_t i = 0; i < m->cap; i++) {
        if (!slot_is_vacant(m->keys[i]))
            rt_array_push(arr, m->keys[i]);
    }
    return arr;
}

RuntimeValue __rt_hashset_is_subset(RuntimeValue a, RuntimeValue b)
{
    HashMap *sa = decode_hashmap(a);
    HashMap *sb = decode_hashmap(b);
    if (!sa || !sb) return FALSE_VALUE;

    /* a is subset of b iff every element of a is in b */
    for (uint32_t i = 0; i < sa->cap; i++) {
        if (slot_is_vacant(sa->keys[i])) continue;
        int found = 0;
        probe_find(sb, sa->keys[i], &found);
        if (!found) return FALSE_VALUE;
    }
    return TRUE_VALUE;
}

RuntimeValue __rt_hashset_is_superset(RuntimeValue a, RuntimeValue b)
{
    /* a is superset of b iff b is subset of a */
    return __rt_hashset_is_subset(b, a);
}

RuntimeValue __rt_hashset_difference(RuntimeValue a, RuntimeValue b)
{
    HashMap *sa = decode_hashmap(a);
    HashMap *sb = decode_hashmap(b);
    if (!sa) return NIL_VALUE;

    RuntimeValue result = __rt_hashset_new();
    if (IS_NIL(result)) return NIL_VALUE;

    for (uint32_t i = 0; i < sa->cap; i++) {
        if (slot_is_vacant(sa->keys[i])) continue;
        int found = 0;
        if (sb) probe_find(sb, sa->keys[i], &found);
        if (!found)
            __rt_hashset_insert(result, sa->keys[i]);
    }
    return result;
}

RuntimeValue __rt_hashset_intersection(RuntimeValue a, RuntimeValue b)
{
    HashMap *sa = decode_hashmap(a);
    HashMap *sb = decode_hashmap(b);
    if (!sa || !sb) return __rt_hashset_new();

    RuntimeValue result = __rt_hashset_new();
    if (IS_NIL(result)) return NIL_VALUE;

    /* Iterate over the smaller set for efficiency */
    HashMap *iter  = (sa->len <= sb->len) ? sa : sb;
    HashMap *check = (sa->len <= sb->len) ? sb : sa;

    for (uint32_t i = 0; i < iter->cap; i++) {
        if (slot_is_vacant(iter->keys[i])) continue;
        int found = 0;
        probe_find(check, iter->keys[i], &found);
        if (found)
            __rt_hashset_insert(result, iter->keys[i]);
    }
    return result;
}

RuntimeValue __rt_hashset_symmetric_difference(RuntimeValue a, RuntimeValue b)
{
    HashMap *sa = decode_hashmap(a);
    HashMap *sb = decode_hashmap(b);

    RuntimeValue result = __rt_hashset_new();
    if (IS_NIL(result)) return NIL_VALUE;

    /* Elements in a but not in b */
    if (sa) {
        for (uint32_t i = 0; i < sa->cap; i++) {
            if (slot_is_vacant(sa->keys[i])) continue;
            int found = 0;
            if (sb) probe_find(sb, sa->keys[i], &found);
            if (!found)
                __rt_hashset_insert(result, sa->keys[i]);
        }
    }

    /* Elements in b but not in a */
    if (sb) {
        for (uint32_t i = 0; i < sb->cap; i++) {
            if (slot_is_vacant(sb->keys[i])) continue;
            int found = 0;
            if (sa) probe_find(sa, sb->keys[i], &found);
            if (!found)
                __rt_hashset_insert(result, sb->keys[i]);
        }
    }

    return result;
}
