#include <onix/types.h>
#include <onix/interrupt.h>
#include <onix/debug.h>
extern void console_init();
extern void gdt_init();
extern void interrupt_init();
extern void clock_init();
extern void time_init();
extern void rtc_init();
extern void set_alarm(u32 secs);
extern void memory_map_init();
extern void mapping_init();
extern void hang();

void intr_test()
{
    bool intr=interrupt_disable();
    set_interrupt_state(intr);
}

void kernel_init()
{
    memory_map_init();
    mapping_init();
    interrupt_init();
    // clock_init();
    // time_init();
    // rtc_init();

    bool intr = interrupt_disable();
    set_interrupt_state(true);
    LOGK("%d\n", intr);
    LOGK("%d\n", get_interrupt_state());

    intr=interrupt_disable();
    set_interrupt_state(true);
    LOGK("%d\n", intr);
    LOGK("%d\n", get_interrupt_state());

    // asm volatile("sti");
    hang();
    return;
}
