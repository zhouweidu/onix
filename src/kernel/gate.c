#include <onix/interrupt.h>
#include <onix/assert.h>
#include <onix/debug.h>
#include <onix/syscall.h>
#include <onix/task.h>
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
    LOGK("sys_test called!!!\n");
    return 255;
}

extern int sys_read(fd_t fd, char *buf, int count);
extern int sys_write(fd_t fd, char *buf, int count);
extern int sys_lseek(fd_t fd, off_t offset, whence_t whence);
extern int sys_readdir(fd_t fd, dirent_t *dir, u32 count);

extern fd_t sys_open(char *filename, int flags, int mode);
extern fd_t sys_creat(char *filename, int mode);
extern fd_t sys_close(fd_t fd);

extern int sys_chdir(char *pathname);
extern int sys_chroot(char *pathname);
extern char *sys_getcwd(char *buf, size_t size);

extern int sys_mkdir(char *pathname, int mode);
extern int sys_rmdir(char *pathname);

extern int sys_link(char *oldname, char *newname);
extern int sys_unlink(char *filename);

extern time_t sys_time();
extern mode_t sys_umask(mode_t mask);

extern int32 sys_brk(void *addr);

extern void console_clear();

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
    syscall_table[SYS_NR_LSEEK] = sys_lseek;
    syscall_table[SYS_NR_READDIR] = sys_readdir;

    syscall_table[SYS_NR_MKDIR] = sys_mkdir;
    syscall_table[SYS_NR_RMDIR] = sys_rmdir;

    syscall_table[SYS_NR_OPEN] = sys_open;
    syscall_table[SYS_NR_CREAT] = sys_creat;
    syscall_table[SYS_NR_CLOSE] = sys_close;

    syscall_table[SYS_NR_LINK] = sys_link;
    syscall_table[SYS_NR_UNLINK] = sys_unlink;

    syscall_table[SYS_NR_TIME] = sys_time;

    syscall_table[SYS_NR_UMASK] = sys_umask;

    syscall_table[SYS_NR_CHDIR] = sys_chdir;
    syscall_table[SYS_NR_CHROOT] = sys_chroot;
    syscall_table[SYS_NR_GETCWD] = sys_getcwd;

    syscall_table[SYS_NR_CLEAR] = console_clear;
}
