#include <onix/interrupt.h>
#include <onix/assert.h>
#include <onix/debug.h>
#include <onix/syscall.h>
#include <onix/task.h>
#include <onix/console.h>
#include <onix/memory.h>
#include <onix/device.h>
#include <onix/string.h>
#include <onix/buffer.h>

#define SYSCALL_SIZE 256

extern int32 console_write(void *dev, char *buf, u32 count);

handler_t syscall_table[SYSCALL_SIZE];

void syscall_check(u32 nr)
{
    if (nr >= SYSCALL_SIZE)
    {
        panic("syscall nr error!!!");
    }
}

static void sys_default()
{
    panic("syscall not implemented!!!");
}

task_t *task = NULL;

static u32 sys_test()
{
    char ch;
    device_t *device;

    // device = device_find(DEV_KEYBOARD, 0);
    // assert(device);
    // device_read(device->dev, &ch, 1, 0, 0);

    // device = device_find(DEV_CONSOLE, 0);
    // assert(device);
    // device_write(device->dev, &ch, 1, 0, 0);

    device = device_find(DEV_IDE_DISK, 0);
    assert(device);
    buffer_t *buf = bread(device->dev, 0);
    char *data=buf->data+SECTOR_SIZE;
    memset(data,0x5a,SECTOR_SIZE);
    buf->dirty=true;
    brelse(buf);
    return 255;
}

int32 sys_write(fd_t fd, char *buf, u32 len)
{
    if (fd == stdout || fd == stderr)
    {
        return console_write(NULL, buf, len);
    }
    panic("write!!!");
    return 0;
}

extern time_t sys_time();

void syscall_init()
{
    for (size_t i = 0; i < SYSCALL_SIZE; i++)
    {
        syscall_table[i] = sys_default;
    }
    syscall_table[SYS_NR_TEST] = sys_test;
    syscall_table[SYS_NR_EXIT] = task_exit;
    syscall_table[SYS_NR_FORK] = task_fork;
    syscall_table[SYS_NR_WAITPID] = task_waitpid;
    syscall_table[SYS_NR_SLEEP] = task_sleep;
    syscall_table[SYS_NR_YIELD] = task_yield;

    syscall_table[SYS_NR_GETPID] = sys_getpid;
    syscall_table[SYS_NR_GETPPID] = sys_getppid;

    syscall_table[SYS_NR_BRK] = sys_brk;

    syscall_table[SYS_NR_WRITE] = sys_write;

    syscall_table[SYS_NR_TIME] = sys_time;
}
