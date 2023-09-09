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
#include <onix/fs.h>

#define SYSCALL_SIZE 256

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
    // char ch;
    // device_t *device;

    // device = device_find(DEV_KEYBOARD, 0);
    // assert(device);
    // device_read(device->dev, &ch, 1, 0, 0);

    // device = device_find(DEV_CONSOLE, 0);
    // assert(device);
    // device_write(device->dev, &ch, 1, 0, 0);

    return 255;
}

extern time_t sys_time();
extern mode_t sys_umask(mode_t mask);

extern int sys_mkdir(char *pathname, int mode);
extern int sys_rmdir(char *pathname);
extern int sys_link(char *oldname, char *newname);
extern int sys_unlink(char *filename);

extern fd_t sys_open(char *filename, int flags, int mode);
extern fd_t sys_creat(char *filename, int mode);
extern fd_t sys_close(fd_t fd);

extern int sys_read(fd_t fd, char *buf, int count);
extern int sys_write(fd_t fd, char *buf, int count);

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

    syscall_table[SYS_NR_READ] = sys_read;
    syscall_table[SYS_NR_WRITE] = sys_write;

    syscall_table[SYS_NR_MKDIR] = sys_mkdir;
    syscall_table[SYS_NR_RMDIR] = sys_rmdir;

    syscall_table[SYS_NR_OPEN] = sys_open;
    syscall_table[SYS_NR_CREAT] = sys_creat;
    syscall_table[SYS_NR_CLOSE] = sys_close;

    syscall_table[SYS_NR_LINK] = sys_link;
    syscall_table[SYS_NR_UNLINK] = sys_unlink;

    syscall_table[SYS_NR_TIME] = sys_time;

    syscall_table[SYS_NR_UMASK] = sys_umask;
}
