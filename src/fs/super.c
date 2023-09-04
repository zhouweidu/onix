#include <onix/fs.h>
#include <onix/buffer.h>
#include <onix/device.h>
#include <onix/assert.h>
#include <onix/string.h>
#include <onix/task.h>
#include <onix/syscall.h>
#include <onix/stdlib.h>
#include <onix/debug.h>

void super_init()
{
    device_t *device = device_find(DEV_IDE_PART, 0);
    assert(device);
    buffer_t *boot = bread(device->dev, 0);
    buffer_t *super = bread(device->dev, 1);
    super_desc_t *sb = (super_desc_t *)super->data;
    assert(sb->magic=MINIX1_MAGIC);
    buffer_t *imap=bread(device->dev,2);
    buffer_t *zmap=bread(device->dev,2);
}