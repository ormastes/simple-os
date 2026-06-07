#include <stdint.h>
typedef void (*simpleos_start_fn)(void);
extern void spl_start(void) __attribute__((weak));
extern void __simple_call_module_inits(void) __attribute__((weak));
int32_t simpleos_startup_has_simple_entry(void){return spl_start?1:0;}
void simpleos_startup_call_module_inits(void){if(__simple_call_module_inits)__simple_call_module_inits();}
void simpleos_startup_call_simple(void){if(spl_start)spl_start();}
void simpleos_startup_handoff(simpleos_start_fn entry){simpleos_startup_call_module_inits();if(entry){entry();return;}simpleos_startup_call_simple();}
