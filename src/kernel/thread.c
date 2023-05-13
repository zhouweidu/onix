#include <onix/interrupt.h>
#include <onix/syscall.h>
#include <onix/debug.h>
#include <onix/mutex.h>
#include <onix/printk.h>

void idle_thread()
{
    set_interrupt_state(true);
    u32 counter = 0;
    while (true)
    {
        // LOGK("idle task.... %d\n", counter++);
        asm volatile(
            "sti\n" // 开中断
            "hlt\n" // 关闭 CPU，进入暂停状态，等待外中断的到来
        );
        yield(); // 放弃执行权，调度执行其他任务
    }
}

extern u32 keyboard_read(char *buf, u32 count);

void init_thread()
{
    set_interrupt_state(true);
    u32 counter = 0;
    char ch;
    while (true)
    {
        bool intr=interrupt_disable();
        keyboard_read(&ch, 1);
        printk("%c",ch);
        // LOGK("init task %d...\n", counter++);
        set_interrupt_state(intr);
    }
}

void test_thread()
{
    set_interrupt_state(true);
    u32 counter = 0;
    while (true)
    {
        // LOGK("test task %d...\n", counter++);
        sleep(709);
    }
}