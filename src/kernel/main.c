#include <onix/types.h>
#include <onix/interrupt.h>
#include <onix/debug.h>

extern void memory_map_init();
extern void mapping_init();
extern void interrupt_init();
extern void clock_init();
extern void time_init();
extern void rtc_init();
extern void keyboard_init();
extern void task_init();
extern void syscall_init();
extern void hang();

void kernel_init()
{
    memory_map_init();
    mapping_init();
    interrupt_init();
    clock_init();
    keyboard_init();
    // time_init();
    // rtc_init();

    task_init();
    syscall_init();
    
    set_interrupt_state(true);
    return;
}
