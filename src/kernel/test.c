#include <onix/types.h>
#include <onix/cpu.h>
#include <onix/printk.h>
#include <onix/debug.h>
#include <onix/errno.h>

extern void test_e1000_send_packet();
err_t sys_test()
{
    test_e1000_send_packet();
    return EOK;
}
