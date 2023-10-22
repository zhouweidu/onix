#include <onix/types.h>
#include <onix/cpu.h>
#include <onix/printk.h>
#include <onix/debug.h>
#include <onix/errno.h>

err_t sys_test()
{
    // LOGK("test syscall. .. \n");

    // cpu_vendor_t vendor;
    // cpu_vendor_id(&vendor);
    // printk("CPU vendor id: %s\n", vendor.info);
    // printk("CPU maxvalue:0x%X\n", vendor.max_value);

    // cpu_version_t ver;
    // cpu_version(&ver);
    // printk("FPU support state: %d\n", ver.FPU);
    // printk("APIC support state:  %d\n", ver.APIC);

    LOGK("%08b\n", 0x5a);
    LOGK("%m\n", "\x11\x22\x03\x04\x5f\x5a");
    LOGK("%r\n", "\xff\x4e\x03\x04");
    return EOK;
}
