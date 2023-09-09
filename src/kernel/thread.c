#include <onix/interrupt.h>
#include <onix/syscall.h>
#include <onix/debug.h>
#include <onix/mutex.h>
#include <onix/printk.h>
#include <onix/task.h>
#include <onix/stdio.h>
#include <onix/arena.h>
#include <onix/stdlib.h>
#include <onix/fs.h>

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

static void user_init_thread()
{
    char buf[256];
    int len = 0;
    fd_t fd = open("/hello.txt", O_RDWR, 0755);
    len = read(fd, buf, sizeof(buf));
    printf("hello.txt content: %s length %d\n", buf, len);
    close(fd);

    fd = open("/world.txt", O_CREAT | O_RDWR, 0755);
    len = write(fd, buf, len);
    while (true)
    {
        char ch;
        read(stdin, &ch, 1);
        write(stdout, &ch, 1);
        sleep(10);
    }
}

void init_thread()
{
    // set_interrupt_state(true);
    // u32 counter = 0;
    // char ch;
    // while (true)
    // {
    //     bool intr=interrupt_disable();
    //     keyboard_read(&ch, 1);
    //     printk("%c",ch);
    //     // LOGK("init task %d...\n", counter++);
    //     set_interrupt_state(intr);
    // }
    char temp[100];
    task_to_user_mode(user_init_thread);
}

void test_thread()
{
    set_interrupt_state(true);
    while (true)
    {
        test();
        sleep(10);
    }
}