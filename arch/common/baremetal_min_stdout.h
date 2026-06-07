#ifndef SIMPLEOS_BAREMETAL_MIN_STDOUT_H
#define SIMPLEOS_BAREMETAL_MIN_STDOUT_H

#include <stdint.h>
#include <stddef.h>

#ifndef BAREMETAL_RUNTIME_H
typedef intptr_t RuntimeValue;

#define TAG_MASK    ((uintptr_t)0x7)
#define TAG_INT     ((uintptr_t)0x0)
#define TAG_HEAP    ((uintptr_t)0x1)
#define ENCODE_INT(v) ((RuntimeValue)(((uintptr_t)(intptr_t)(v) << 3) | TAG_INT))
#define DECODE_PTR(v) ((void *)((uintptr_t)(v) & ~TAG_MASK))
#define IS_HEAP(v)    (((uintptr_t)(v) & TAG_MASK) == TAG_HEAP)
#define HEAP_STRING 1u

typedef struct{uint32_t type,size;} BaremetalHeapHeader;

typedef struct{BaremetalHeapHeader hdr;uint32_t len;char data[];} BaremetalRuntimeString;
#else
typedef RuntimeString BaremetalRuntimeString;
#endif

static RuntimeValue baremetal_serial_write_value(RuntimeValue data){
    if (IS_HEAP(data)) {
        BaremetalRuntimeString *s = (BaremetalRuntimeString *)DECODE_PTR(data);
        if (s && s->hdr.type == HEAP_STRING && s->len < 0x100000u) {
            for (uint32_t i = 0; i < s->len; i++) serial_putchar(s->data[i]);
            return ENCODE_INT((intptr_t)s->len);
        }
    }
    return ENCODE_INT(0);
}

RuntimeValue rt_stdout_write(RuntimeValue data){return baremetal_serial_write_value(data);}

RuntimeValue rt_stdout_flush(RuntimeValue data){(void)data;return ENCODE_INT(0);}

RuntimeValue rt_stderr_write(RuntimeValue data) __attribute__((alias("rt_stdout_write")));
RuntimeValue rt_stderr_flush(RuntimeValue data) __attribute__((alias("rt_stdout_flush")));

#endif
