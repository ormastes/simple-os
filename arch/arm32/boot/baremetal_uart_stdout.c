#define PL011_BASE 0x09000000u
#define PL011_IBRD_VALUE 13u
#define PL011_FBRD_VALUE 1u
#define SIMPLEOS_CALL_MODULE_INITS 0
#define SIMPLEOS_PL011_ENTRY _start
#define SIMPLEOS_PL011_HALT "wfi"
#include "../../common/baremetal_pl011_uart_stdout.c"
