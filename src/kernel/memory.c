#include <onix/memory.h>
#include <onix/types.h>
#include <onix/debug.h>
#include <onix/assert.h>
#include <onix/stdlib.h>
#include <onix/string.h>
#include <onix/bitmap.h>
#include <onix/task.h>

#define ZONE_VALID 1    // ards 可用内存区域
#define ZONE_RESERVED 2 // ards 不可用区域

#define IDX(addr) ((u32)addr >> 12)            // 获取 addr 的页索引
#define DIDX(addr) (((u32)addr >> 22) & 0x3ff) // 获取 addr 的页目录索引
#define TIDX(addr) (((u32)addr >> 12) & 0x3ff) // 获取 addr 的页表索引
#define PAGE(idx) ((u32)idx << 12)             // 获取页索引 idx 对应的页开始的位置
#define ASSERT_PAGE(addr) assert((addr & 0xfff) == 0)

#define KERNEL_MAP_BITS 0x4000

#define PDE_MASK 0xffc00000

bitmap_t kernel_map;

typedef struct ards_t
{
    u64 base; // 内存基地址
    u64 size; // 内存长度
    u32 type; // 类型
} _packed ards_t;

static u32 memory_base = 0; // 可用内存基地址，应该等于 1M
static u32 memory_size = 0; // 可用内存大小
static u32 total_pages = 0; // 所有内存页数
static u32 free_pages = 0;  // 空闲内存页数

#define used_pages (total_pages - free_pages) // 已用页数

void memory_init(u32 magic, u32 addr)
{
    u32 count = 0;
    ards_t *ptr;

    // 如果是 onix loader 进入的内核
    if (magic == ONIX_MAGIC)
    {
        count = *(u32 *)addr;
        ptr = (ards_t *)(addr + 4);

        for (size_t i = 0; i < count; i++, ptr++)
        {
            LOGK("Memory base 0x%p size 0x%p type %d\n",
                 (u32)ptr->base, (u32)ptr->size, (u32)ptr->type);
            if (ptr->type == ZONE_VALID && ptr->size > memory_size)
            {
                memory_base = (u32)ptr->base;
                memory_size = (u32)ptr->size;
            }
        }
    }
    else
    {
        panic("Memory init magic unknown 0x%p\n", magic);
    }

    LOGK("ARDS count %d\n", count);
    LOGK("Memory base 0x%p\n", (u32)memory_base);
    LOGK("Memory size 0x%p\n", (u32)memory_size);

    assert(memory_base == MEMORY_BASE); // 内存开始的位置为 1M
    assert((memory_size & 0xfff) == 0); // 要求按页对齐

    total_pages = IDX(memory_size) + IDX(MEMORY_BASE);
    free_pages = IDX(memory_size);

    LOGK("Total pages %d\n", total_pages);
    LOGK("Free pages %d\n", free_pages);

    if (memory_size < KERNEL_MEMORY_SIZE)
    {
        panic("System memory is %dM too small, at least %dM needed\n",
              memory_size / MEMORY_BASE, KERNEL_MEMORY_SIZE / MEMORY_BASE);
    }
}

static u32 start_page = 0;   // 可分配物理内存起始位置，起始页号
static u8 *memory_map;       // 物理内存数组用来管理每一个页
static u32 memory_map_pages; // 物理内存数组占用的页数

void memory_map_init()
{
    // 初始化物理内存数组
    memory_map = (u8 *)memory_base;

    // 计算物理内存数组占用的页数，占用了2页
    memory_map_pages = div_round_up(total_pages, PAGE_SIZE);
    LOGK("Memory map page count %d\n", memory_map_pages);

    free_pages -= memory_map_pages;

    // 清空物理内存数组
    memset((void *)memory_map, 0, memory_map_pages * PAGE_SIZE);

    // 前 1M 的内存位置 以及 物理内存数组已占用的页，已被占用
    start_page = IDX(MEMORY_BASE) + memory_map_pages;
    for (size_t i = 0; i < start_page; i++)
    {
        memory_map[i] = 1;
    }

    LOGK("Total pages %d free pages %d\n", total_pages, free_pages);

    // 初始化内核虚拟内存位图，需要 8 位对齐，是针对页的索引（页号）的位图，也就是高20位
    u32 length = (IDX(KERNEL_MEMORY_SIZE) - IDX(MEMORY_BASE)) / 8;
    bitmap_init(&kernel_map, (u8 *)KERNEL_MAP_BITS, length, IDX(MEMORY_BASE));
    bitmap_scan(&kernel_map, memory_map_pages);
}

// 分配一页物理内存
static u32 get_page()
{
    for (size_t i = start_page; i < total_pages; i++)
    {
        // 如果物理内存没有占用
        if (!memory_map[i])
        {
            memory_map[i] = 1;
            assert(free_pages > 0);
            free_pages--;
            u32 page = PAGE(i);
            LOGK("GET page 0x%p\n", page);
            return page;
        }
    }
    panic("Out of Memory!!!");
}

// 释放一页物理内存
static void put_page(u32 addr)
{
    ASSERT_PAGE(addr);

    u32 idx = IDX(addr);

    // idx 大于 1M 并且 小于 总页面数
    assert(idx >= start_page && idx < total_pages);

    // 保证至少有一个引用
    assert(memory_map[idx] >= 1);

    // 物理引用减一
    memory_map[idx]--;

    // 若为 0，则空闲页加一
    if (!memory_map[idx])
    {
        free_pages++;
    }

    assert(free_pages > 0 && free_pages < total_pages);
    LOGK("PUT page 0x%p\n", addr);
}

u32 get_cr2()
{
    asm volatile("movl %cr2, %eax\n");
}

u32 get_cr3()
{
    asm volatile("movl %cr3, %eax\n");
}

void set_cr3(u32 pde)
{
    ASSERT_PAGE(pde);
    asm volatile("movl %%eax, %%cr3\n" ::"a"(pde));
}

// 将 cr0 寄存器最高位 PG 置为 1，启用分页
static _inline void enable_page()
{
    // 0b1000_0000_0000_0000_0000_0000_0000_0000
    // 0x80000000
    asm volatile(
        "movl %cr0, %eax\n"
        "orl $0x80000000, %eax\n"
        "movl %eax, %cr0\n");
}

// 初始化页表项
static void entry_init(page_entry_t *entry, u32 index)
{
    *(u32 *)entry = 0;
    entry->present = 1;
    entry->write = 1;
    entry->user = 1;
    entry->index = index;
}

// 初始化内存映射
void mapping_init()
{
    page_entry_t *pde = (page_entry_t *)KERNEL_PAGE_DIR;
    memset(pde, 0, PAGE_SIZE);

    idx_t index = 0;

    for (idx_t didx = 0; didx < (sizeof(KERNEL_PAGE_TABLE) / 4); didx++)
    {
        page_entry_t *pte = (page_entry_t *)KERNEL_PAGE_TABLE[didx];
        memset(pte, 0, PAGE_SIZE);

        page_entry_t *dentry = &pde[didx];
        entry_init(dentry, IDX((u32)pte));

        for (idx_t tidx = 0; tidx < 1024; tidx++, index++)
        {
            // 第 0 页不映射，为造成空指针访问，缺页异常，便于排错
            if (index == 0)
                continue;

            page_entry_t *tentry = &pte[tidx];
            entry_init(tentry, index);
            memory_map[index] = 1; // 设置物理内存数组，该页被占用
        }
    }

    // 将最后一个页表指向页目录自己，方便修改
    page_entry_t *entry = &pde[1023];
    entry_init(entry, IDX(KERNEL_PAGE_DIR));

    // 设置 cr3 寄存器
    set_cr3((u32)pde);

    // 分页有效
    enable_page();
}

// 获取页目录
static page_entry_t *get_pde()
{
    return (page_entry_t *)(0xfffff000);
}

// 获取虚拟地址 vaddr 对应的页表
static page_entry_t *get_pte(u32 vaddr, bool create)
{
    page_entry_t *pde = get_pde();
    u32 idx = DIDX(vaddr);
    page_entry_t *entry = &pde[idx];

    assert(create || (!create && entry->present));

    page_entry_t *table = (page_entry_t *)(PDE_MASK | (idx << 12));

    if (!entry->present)
    {
        LOGK("Get and create page table entry for 0x%p\n", vaddr);
        u32 page = get_page();
        entry_init(entry, IDX(page));
        memset(table, 0, PAGE_SIZE);
    }

    return table;
}

// 刷新虚拟地址 vaddr 的 快表 TLB
static void flush_tlb(u32 vaddr)
{
    asm volatile("invlpg (%0)" ::"r"(vaddr)
                 : "memory");
}

// 从位图中扫描 count 个连续的页
static u32 scan_page(bitmap_t *map, u32 count)
{
    assert(count > 0);
    int32 index = bitmap_scan(map, count);

    if (index == EOF)
    {
        panic("Scan page fail!!!");
    }

    u32 addr = PAGE(index);
    LOGK("Scan page 0x%p count %d\n", addr, count);
    return addr;
}

// 与 scan_page 相对，重置相应的页
static void reset_page(bitmap_t *map, u32 addr, u32 count)
{
    ASSERT_PAGE(addr);
    assert(count > 0);
    u32 index = IDX(addr);

    for (size_t i = 0; i < count; i++)
    {
        assert(bitmap_test(map, index + i));
        bitmap_set(map, index + i, 0);
    }
}

// 分配 count 个连续的内核页
u32 alloc_kpage(u32 count)
{
    assert(count > 0);
    u32 vaddr = scan_page(&kernel_map, count);
    LOGK("ALLOC kernel pages 0x%p count %d\n", vaddr, count);
    return vaddr;
}

// 释放 count 个连续的内核页
void free_kpage(u32 vaddr, u32 count)
{
    ASSERT_PAGE(vaddr);
    assert(count > 0);
    reset_page(&kernel_map, vaddr, count);
    LOGK("FREE  kernel pages 0x%p count %d\n", vaddr, count);
}

page_entry_t *get_entry(u32 vaddr, bool create)
{
    page_entry_t *pte = get_pte(vaddr, create);
    return &pte[TIDX(vaddr)];
}

// 将 vaddr 映射物理内存
void link_page(u32 vaddr)
{
    ASSERT_PAGE(vaddr);

    page_entry_t *entry = get_entry(vaddr, true);

    task_t *task = running_task();
    bitmap_t *map = task->vmap;
    u32 index = IDX(vaddr);

    // 如果页面已存在，则直接返回
    if (entry->present)
    {
        assert(bitmap_test(map, index));
        return;
    }

    assert(!bitmap_test(map, index));
    bitmap_set(map, index, true);
    u32 paddr = get_page();
    entry_init(entry, IDX(paddr));
    flush_tlb(vaddr);

    LOGK("LINK from 0x%p to 0x%p\n", vaddr, paddr);
}

// 去掉 vaddr 对应的物理内存映射
void unlink_page(u32 vaddr)
{
    ASSERT_PAGE(vaddr);

    page_entry_t *pte = get_pte(vaddr, true);
    page_entry_t *entry = &pte[TIDX(vaddr)];

    task_t *task = running_task();
    bitmap_t *map = task->vmap;
    u32 index = IDX(vaddr);

    if (!entry->present)
    {
        assert(!bitmap_test(map, index));
        return;
    }

    assert(entry->present && bitmap_test(map, index));

    entry->present = false;
    bitmap_set(map, index, false);

    u32 paddr = PAGE(entry->index);

    DEBUGK("UNLINK from 0x%p to 0x%p\n", vaddr, paddr);

    put_page(paddr);

    flush_tlb(vaddr);
}

page_entry_t *copy_pde()
{
    task_t *task = running_task();
    page_entry_t *pde = (page_entry_t *)alloc_kpage(1);
    memcpy(pde, (void *)task->pde, PAGE_SIZE);
    page_entry_t *entry = &pde[1023];
    entry_init(entry, IDX(pde));
    return pde;
}

typedef struct page_error_code_t
{
    u8 present : 1;
    u8 write : 1;
    u8 user : 1;
    u8 reserved0 : 1;
    u8 fetch : 1;
    u8 protection : 1;
    u8 shadow : 1;
    u16 reserved1 : 8;
    u8 sgx : 1;
    u16 reserved2;
} _packed page_error_code_t;

void page_fault(
    u32 vector,
    u32 edi, u32 esi, u32 ebp, u32 esp,
    u32 ebx, u32 edx, u32 ecx, u32 eax,
    u32 gs, u32 fs, u32 es, u32 ds,
    u32 vector0, u32 error, u32 eip, u32 cs, u32 eflags)
{
    assert(vector == 0xe);
    u32 vaddr = get_cr2();
    LOGK("fault address 0x%p\n", vaddr);

    page_error_code_t *code = (page_error_code_t *)&error;
    task_t *task = running_task();

    assert(KERNEL_MEMORY_SIZE <= vaddr && vaddr < USER_STACK_TOP);

    // 如果用户程序访问了不该访问的内存
    if (!code->present && vaddr > USER_STACK_BOTTOM)
    {
        u32 page = PAGE(IDX(vaddr));
        link_page(page);
        return;
    }
    panic("page fault!!!");
}
