#ifndef ONIX_TASK_H
#define ONIX_TASK_H

#include <onix/types.h>

typedef u32 target_t(); //调用函数的入口地址

typedef struct task_t
{
    u32 *stack; // 内核栈，保存进程栈顶在哪
} task_t;

typedef struct task_frame_t
{
    u32 edi;
    u32 esi;
    u32 ebx;
    u32 ebp;
    void (*eip)(void);
} task_frame_t;

void task_init();

#endif