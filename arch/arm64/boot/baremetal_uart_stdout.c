#define PL011_BASE 0x09000000ULL
#define PL011_IBRD_VALUE 1u
#define PL011_FBRD_VALUE 0u
#define SIMPLEOS_CALL_MODULE_INITS 1
#define SIMPLEOS_PL011_ENTRY _c_start
#define SIMPLEOS_PL011_HALT "wfe"
#include "../../common/baremetal_pl011_uart_stdout.c"
