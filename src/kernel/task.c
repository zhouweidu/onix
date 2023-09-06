#include <onix/task.h>
#include <onix/printk.h>
#include <onix/debug.h>
#include <onix/memory.h>
#include <onix/assert.h>
#include <onix/interrupt.h>
#include <onix/string.h>
#include <onix/bitmap.h>
#include <onix/syscall.h>
#include <onix/global.h>
#include <onix/arena.h>
#include <onix/fs.h>

#define NR_TASKS 64

extern u32 volatile jiffies;
extern u32 jiffy;
extern bitmap_t kernel_map;
extern void task_switch(task_t *next);
extern tss_t tss;

static task_t *task_table[NR_TASKS]; // 任务表
static list_t block_list;            // 任务默认阻塞链表
static list_t sleep_list;
static task_t *idle_task;

// 从 task_table 里获得一个空闲的任务
static task_t *get_free_task()
{
    for (size_t i = 0; i < NR_TASKS; i++)
    {
        if (task_table[i] == NULL)
        {
            task_t *task = (task_t *)alloc_kpage(1);
            memset(task, 0, PAGE_SIZE);
            task->pid = i;
            task_table[i] = task;
            return task;
        }
    }
    panic("No more tasks");
}

pid_t sys_getpid()
{
    task_t *task = running_task();
    return task->pid;
}

pid_t sys_getppid()
{
    task_t *task = running_task();
    return task->ppid;
}

// 从任务数组中查找某种状态的任务，自己除外
static task_t *task_search(task_state_t state)
{
    assert(!get_interrupt_state());
    task_t *task = NULL;
    task_t *current = running_task();

    for (size_t i = 0; i < NR_TASKS; i++)
    {
        task_t *ptr = task_table[i];
        if (ptr == NULL)
            continue;

        if (ptr->state != state)
            continue;
        if (current == ptr)
            continue;
        if (task == NULL || task->ticks < ptr->ticks || ptr->jiffies < task->jiffies)
            task = ptr;
    }
    if (task == NULL && state == TASK_READY)
    {
        task = idle_task;
    }

    return task;
}

void task_yield()
{
    schedule();
}

// 任务阻塞
void task_block(task_t *task, list_t *blist, task_state_t state)
{
    assert(!get_interrupt_state());
    assert(task->node.next == NULL);
    assert(task->node.prev == NULL);

    if (blist == NULL)
    {
        blist = &block_list;
    }

    assert(state != TASK_READY && state != TASK_RUNNING);

    list_push(blist, &task->node);

    task->state = state;

    task_t *current = running_task();
    if (current == task)
    {
        schedule();
    }
}

// 解除任务阻塞
void task_unblock(task_t *task)
{
    assert(!get_interrupt_state());

    list_remove(&task->node);

    assert(task->node.next == NULL);
    assert(task->node.prev == NULL);

    task->state = TASK_READY;
}

void task_sleep(u32 ms)
{
    assert(!get_interrupt_state()); // 不可中断

    u32 ticks = ms / jiffy;
    ticks = ticks > 0 ? ticks : 1;

    task_t *current = running_task();
    current->ticks = jiffies + ticks;

    list_insert_sort(&sleep_list, &current->node, element_node_offset(task_t, node, ticks));
    current->state = TASK_SLEEPING;
    schedule();
}

void task_wakeup()
{
    assert(!get_interrupt_state());
    list_t *list = &sleep_list;
    for (list_node_t *ptr = list->head.next; ptr != &list->tail;)
    {
        task_t *task = element_entry(task_t, node, ptr);
        if (task->ticks > jiffies)
        {
            break;
        }
        ptr = ptr->next;
        task->ticks = 0;
        task_unblock(task);
    }
}

// 激活任务
void task_activate(task_t *task)
{
    assert(task->magic == ONIX_MAGIC);

    if (task->pde != get_cr3())
    {
        set_cr3(task->pde);
    }

    if (task->uid != KERNEL_USER)
    {
        tss.esp0 = (u32)task + PAGE_SIZE;
    }
}

task_t *running_task()
{
    asm volatile(
        "movl %esp, %eax\n"
        "andl $0xfffff000, %eax\n");
}

void schedule()
{
    assert(!get_interrupt_state());
    task_t *current = running_task();
    task_t *next = task_search(TASK_READY);

    assert(next != NULL);
    assert(next->magic == ONIX_MAGIC);

    if (current->state == TASK_RUNNING)
    {
        current->state = TASK_READY;
    }
    if (!current->ticks)
    {
        current->ticks = current->priority;
    }

    next->state = TASK_RUNNING;
    if (next == current)
        return;

    task_activate(next);
    task_switch(next);
}

static task_t *task_create(target_t target, const char *name, u32 priority, u32 uid)
{
    task_t *task = get_free_task();

    u32 stack = (u32)task + PAGE_SIZE;

    stack -= sizeof(task_frame_t);
    task_frame_t *frame = (task_frame_t *)stack;
    frame->ebx = 0x11111111;
    frame->esi = 0x22222222;
    frame->edi = 0x33333333;
    frame->ebp = 0x44444444;
    frame->eip = (void *)target;

    strcpy((char *)task->name, name);

    task->stack = (u32 *)stack;
    task->priority = priority;
    task->ticks = task->priority;
    task->jiffies = 0;
    task->state = TASK_READY;
    task->uid = uid;
    task->vmap = &kernel_map;
    task->pde = KERNEL_PAGE_DIR;
    task->brk = KERNEL_MEMORY_SIZE;
    task->iroot=get_root_inode();
    task->ipwd=get_root_inode();

    task->magic = ONIX_MAGIC;
    return task;
}

// 调用该函数的地方不能有任何局部变量
// 调用前栈顶需要准备足够的空间
void task_to_user_mode(target_t target)
{
    task_t *task = running_task();

    task->vmap = kmalloc(sizeof(bitmap_t));
    // 一页内存按位算只能表示128MB的空间
    void *buf = (void *)alloc_kpage(1);
    bitmap_init(task->vmap, buf, PAGE_SIZE, KERNEL_MEMORY_SIZE / PAGE_SIZE);

    task->pde = (u32)copy_pde();
    set_cr3(task->pde);

    u32 addr = (u32)task + PAGE_SIZE;

    addr -= sizeof(intr_frame_t);
    intr_frame_t *iframe = (intr_frame_t *)(addr);

    iframe->vector = 0x20;
    iframe->edi = 1;
    iframe->esi = 2;
    iframe->ebp = 3;
    iframe->esp_dummy = 4;
    iframe->ebx = 5;
    iframe->edx = 6;
    iframe->ecx = 7;
    iframe->eax = 8;

    iframe->gs = 0;
    iframe->ds = USER_DATA_SELECTOR;
    iframe->es = USER_DATA_SELECTOR;
    iframe->fs = USER_DATA_SELECTOR;
    iframe->ss = USER_DATA_SELECTOR;
    iframe->cs = USER_CODE_SELECTOR;

    iframe->error = ONIX_MAGIC;

    iframe->eip = (u32)target;
    // 开中断，IOPL为0，io只有在特权级0才能被执行，用户模式in out指令会报错
    iframe->eflags = (0 << 12 | 0b10 | 1 << 9);
    // esp指向的是用户栈，stack3是用户栈，task+PAGE_SIZE是内核栈
    iframe->esp = USER_STACK_TOP;
    asm volatile(
        "movl %0, %%esp\n"
        "jmp interrupt_exit\n" ::"m"(iframe));
}

extern void interrupt_exit();

static void task_build_stack(task_t *task)
{
    u32 addr = (u32)task + PAGE_SIZE;
    addr -= sizeof(intr_frame_t);
    intr_frame_t *iframe = (intr_frame_t *)addr;
    iframe->eax = 0;

    addr -= sizeof(task_frame_t);
    task_frame_t *frame = (task_frame_t *)addr;

    frame->ebp = 0xaa55aa55;
    frame->ebx = 0xaa55aa55;
    frame->edi = 0xaa55aa55;
    frame->esi = 0xaa55aa55;

    frame->eip = interrupt_exit;

    task->stack = (u32 *)frame;
}

pid_t task_fork()
{
    // LOGK("fork is called\n");
    task_t *task = running_task();

    // 当前进程没有阻塞，且正在执行
    assert(task->node.next == NULL && task->node.prev == NULL && task->state == TASK_RUNNING);

    // 拷贝内核栈 和 PCB
    task_t *child = get_free_task();
    pid_t pid = child->pid;
    memcpy(child, task, PAGE_SIZE);

    child->pid = pid;
    child->ppid = task->pid;

    child->ticks = child->priority;
    child->state = TASK_READY;

    // 拷贝用户进程虚拟内存位图
    child->vmap = kmalloc(sizeof(bitmap_t));
    memcpy(child->vmap, task->vmap, sizeof(bitmap_t));

    // 拷贝虚拟位图缓存
    void *buf = (void *)alloc_kpage(1);
    memcpy(buf, task->vmap->bits, PAGE_SIZE);
    child->vmap->bits = buf;

    // 拷贝页目录
    child->pde = (u32)copy_pde();

    // 构造 child 内核栈
    task_build_stack(child); // ROP
    // schedule();

    return child->pid;
}

void task_exit(int status)
{
    task_t *task = running_task();

    // 当前进程没有阻塞，且正在执行
    assert(task->node.next == NULL && task->node.prev == NULL && task->state == TASK_RUNNING);

    task->state = TASK_DIED;
    task->status = status;

    free_pde();

    free_kpage((u32)task->vmap->bits, 1);
    kfree(task->vmap);

    // 将子进程的父进程赋值为自己的父进程
    for (size_t i = 2; i < NR_TASKS; i++)
    {
        task_t *child = task_table[i];
        if (!child)
            continue;
        if (child->ppid != task->pid)
            continue;
        child->ppid = task->ppid;
    }
    LOGK("task 0x%p exit....\n", task);

    task_t *parent = task_table[task->ppid];
    if (parent->state == TASK_WAITING &&
        (parent->waitpid == -1 || parent->waitpid == task->pid))
    {
        task_unblock(parent);
    }
    schedule();
}

pid_t task_waitpid(pid_t pid, int32 *status)
{
    // 如果pid=-1就wait任何一个子进程，如果是特定的pid就wait特定的pid
    task_t *task = running_task();
    task_t *child = NULL;

    while (true)
    {
        bool has_child = false;
        for (size_t i = 2; i < NR_TASKS; i++)
        {
            task_t *ptr = task_table[i];
            if (!ptr)
                continue;

            if (ptr->ppid != task->pid)
                continue;
            if (pid != ptr->pid && pid != -1)
                continue;

            if (ptr->state == TASK_DIED)
            {
                child = ptr;
                task_table[i] = NULL;
                goto rollback;
            }

            has_child = true;
        }
        if (has_child)
        {
            task->waitpid = pid;
            task_block(task, NULL, TASK_WAITING);
            continue;
        }
        break;
    }

    // 没找到符合条件的子进程
    return -1;

rollback:
    *status = child->status;
    u32 ret = child->pid;
    free_kpage((u32)child, 1);
    return ret;
}

static void task_setup()
{
    task_t *task = running_task();
    task->magic = ONIX_MAGIC;
    task->ticks = 1;

    memset(task_table, 0, sizeof(task_table));
}

extern void idle_thread();
extern void init_thread();
extern void test_thread();

void task_init()
{
    list_init(&block_list);
    list_init(&sleep_list);
    task_setup();
    idle_task = task_create(idle_thread, "idle", 1, KERNEL_USER);
    task_create(init_thread, "init", 5, NORMAL_USER);
    task_create(test_thread, "test", 5, KERNEL_USER);
}