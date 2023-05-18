#ifndef ONIX_MEMORY_H
#define ONIX_MEMORY_H

#include <onix/types.h>

// 内核页表索引
static u32 KERNEL_PAGE_TABLE[] = {
    0x2000,
    0x3000,
};

#define PAGE_SIZE 0x1000     // 一页的大小，4K
#define MEMORY_BASE 0x100000 // 1M 可用内存的开始位置

#define KERNEL_PAGE_DIR 0x1000

#define KERNEL_MEMORY_SIZE (0x100000 * sizeof(KERNEL_PAGE_TABLE))

#define USER_STACK_TOP 0x8000000

#define USER_STACK_SIZE 0x200000

#define USER_STACK_BOTTOM (USER_STACK_TOP - USER_STACK_SIZE)

typedef struct page_entry_t
{
    u8 present : 1;  // 在内存中
    u8 write : 1;    // 0 只读 1 可读可写
    u8 user : 1;     // 1 所有人 0 超级用户 DPL < 3
    u8 pwt : 1;      // page write through 1 直写模式，0 回写模式
    u8 pcd : 1;      // page cache disable 禁止该页缓冲
    u8 accessed : 1; // 被访问过，用于统计使用频率
    u8 dirty : 1;    // 脏页，表示该页缓冲被写过
    u8 pat : 1;      // page attribute table 页大小 4K/4M
    u8 global : 1;   // 全局，所有进程都用到了，该页不刷新缓冲
    u8 shared : 1;   // 共享内存页，与 CPU 无关
    u8 privat : 1;   // 私有内存页，与 CPU 无关
    u8 readonly : 1; // 只读内存页，与 CPU 无关
    u32 index : 20;  // 页索引
} _packed page_entry_t;

u32 get_cr2();
u32 get_cr3();
void set_cr3(u32 pde);

// 分配 count 个连续的内核页
u32 alloc_kpage(u32 count);

// 释放 count 个连续的内核页
void free_kpage(u32 vaddr, u32 count);

void link_page(u32 vaddr);

void unlink_page(u32 vaddr);

// 拷贝页目录
page_entry_t *copy_pde();

int32 sys_brk(void *addr);
#endif
