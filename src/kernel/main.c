#include <onix/interrupt.h>

extern void tss_init();
extern void memory_map_init();
extern void mapping_init();
extern void arena_init();

extern void interrupt_init();
extern void clock_init();
extern void timer_init();
extern void syscall_init();
extern void task_init();
extern void fpu_init();
extern void pci_init();

extern void pbuf_init();
extern void netif_init();
extern void loopif_init();
extern void eth_init();
extern void arp_init();
extern void ip_init();
extern void icmp_init();

void kernel_init()
{
    tss_init();        // 初始化任务状态段
    memory_map_init(); // 初始化物理内存数组
    mapping_init();    // 初始化内存映射
    arena_init();      // 初始化内核堆内存

    interrupt_init(); // 初始化中断
    timer_init();     // 初始化定时器
    clock_init();     // 初始化时钟
    fpu_init();       // 初始化FPU
    pci_init();       // 初始化PCI总线

    syscall_init(); // 初始化系统调用
    task_init();    // 初始化任务

    pbuf_init();
    netif_init();
    loopif_init();
    eth_init();
    arp_init();
    ip_init();
    icmp_init();

    set_interrupt_state(true);
}
