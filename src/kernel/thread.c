#include <onix/interrupt.h>
#include <onix/syscall.h>
#include <onix/debug.h>
#include <onix/mutex.h>
#include <onix/printk.h>
#include <onix/task.h>
#include <onix/stdio.h>
#include <onix/arena.h>
#include <onix/stdlib.h>

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
    u32 counter = 0;
    int status;
    while (true)
    {
        // pid_t pid = fork();
        // if (pid)
        // {
        //     printf("fork after parent %d %d %d\n", pid, getpid(), getppid());
        //     // sleep(1000);
        //     pid_t child = waitpid(pid, &status);
        //     printf("wait pid %d status %d %d\n", child, status, time());
        // }
        // else
        // {
        //     printf("fork after child %d %d %d\n", pid, getpid(), getppid());
        //     sleep(1000);
        //     exit(0);
        // }
        sleep(1000);
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