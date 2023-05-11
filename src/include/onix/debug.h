#ifndef ONIX_DEBUG_H
#define ONIX_DEBUG_H

void debugk(char *file, int line, const char *fmt, ...);

#define BMB asm volatile("xchgw %bx, %bx") // bochs magic breakpoint，bochs的环境没配好，所以无法使用
#define DEBUGK(fmt, args...) debugk(__BASE_FILE__, __LINE__, fmt, ##args)

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)
//将宏定义为下面这种就把调试信息关掉了
// #define LOGK(fmt, args...)


#endif
