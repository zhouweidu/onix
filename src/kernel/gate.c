#include <onix/interrupt.h>
#include <onix/assert.h>
#include <onix/debug.h>
#include <onix/syscall.h>
#include <onix/task.h>
#include <onix/fs.h>
#include <onix/device.h>

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
    // LOGK("sys_test called!!!\n");
    return 255;
}

extern int sys_execve(char *filename, char *argv[], char *envp[]);

extern fd_t sys_dup(fd_t oldfd);
extern fd_t sys_dup2(fd_t oldfd, fd_t newfd);

extern int sys_pipe(fd_t pipefd[2]);

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

extern int sys_stat(char *filename, stat_t *statbuf);
extern int sys_fstat(fd_t fd, stat_t *statbuf);

extern int sys_mknod(char *filename, int mode, int dev);

extern int sys_mount(char *devname, char *dirname, int flags);
extern int sys_umount(char *target);

extern int32 sys_brk(void *addr);
extern int sys_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
extern int sys_munmap(void *addr, size_t length);

extern int sys_setpgid(int pid, int pgid);
extern int sys_setsid();
extern int sys_getpgrp();

extern int sys_stty();
extern int sys_gtty();
extern int sys_ioctl();

extern int sys_mkfs(char *devname, int icount);

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

    syscall_table[SYS_NR_EXECVE] = sys_execve;

    syscall_table[SYS_NR_SLEEP] = task_sleep;
    syscall_table[SYS_NR_YIELD] = task_yield;

    syscall_table[SYS_NR_GETPID] = sys_getpid;
    syscall_table[SYS_NR_GETPPID] = sys_getppid;

    syscall_table[SYS_NR_SETPGID] = sys_setpgid;
    syscall_table[SYS_NR_GETPGRP] = sys_getpgrp;
    syscall_table[SYS_NR_SETSID] = sys_setsid;

    syscall_table[SYS_NR_STTY] = sys_stty;
    syscall_table[SYS_NR_GTTY] = sys_gtty;
    syscall_table[SYS_NR_IOCTL] = sys_ioctl;

    syscall_table[SYS_NR_BRK] = sys_brk;
    syscall_table[SYS_NR_MMAP] = sys_mmap;
    syscall_table[SYS_NR_MUNMAP] = sys_munmap;

    syscall_table[SYS_NR_DUP] = sys_dup;
    syscall_table[SYS_NR_DUP2] = sys_dup2;

    syscall_table[SYS_NR_PIPE] = sys_pipe;

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

    syscall_table[SYS_NR_STAT] = sys_stat;
    syscall_table[SYS_NR_FSTAT] = sys_fstat;

    syscall_table[SYS_NR_MKNOD] = sys_mknod;

    syscall_table[SYS_NR_MOUNT] = sys_mount;
    syscall_table[SYS_NR_UMOUNT] = sys_umount;

    syscall_table[SYS_NR_MKFS] = sys_mkfs;
}
