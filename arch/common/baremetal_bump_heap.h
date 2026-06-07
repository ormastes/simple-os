#ifndef SIMPLEOS_BAREMETAL_BUMP_HEAP_H
#define SIMPLEOS_BAREMETAL_BUMP_HEAP_H

static void *rv_alloc(size_t size){
    size = (size + 15U) & ~(size_t)15U;
    if (g_heap_off + size > sizeof(g_heap)) return 0;
    void *p = &g_heap[g_heap_off];
    g_heap_off += size;
    return p;
}

#ifdef BAREMETAL_ENABLE_ALIGNED_ALLOC
static void *rv_alloc_aligned(size_t size, size_t align){
    if (align == 0) align = 16U;
    size_t offset = g_heap_off;
    size_t rem = offset % align;
    if (rem != 0) offset += align - rem;
    size = (size + 15U) & ~(size_t)15U;
    if (offset + size > sizeof(g_heap)) return 0;
    void *p = &g_heap[offset];
    g_heap_off = offset + size;
    return p;
}
#endif

#endif
