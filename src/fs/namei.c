#include <onix/fs.h>
#include <onix/buffer.h>
#include <onix/stat.h>
#include <onix/syscall.h>
#include <onix/string.h>
#include <onix/task.h>
#include <onix/assert.h>
#include <onix/debug.h>
#include <onix/memory.h>

bool permission(inode_t *inode, u16 mask)
{
    u16 mode = inode->desc->mode;

    if (!inode->desc->nlinks)
        return false;

    task_t *task = running_task();
    if (task->uid == KERNEL_USER)
        return true;

    if (task->uid == inode->desc->uid)
        mode >>= 6;
    else if (task->gid == inode->desc->gid)
        mode >>= 3;

    if ((mode & mask & 0b111) == mask)
        return true;
    return false;
}

// 判断文件名是否相等
static bool match_name(const char *name, const char *entry_name, char **next)
{
    char *lhs = (char *)name;
    char *rhs = (char *)entry_name;
    while (*lhs == *rhs && *lhs != EOS && *rhs != EOS)
    {
        lhs++;
        rhs++;
    }
    if (*rhs)
        return false;
    if (*lhs && !IS_SEPARATOR(*lhs))
        return false;
    if (IS_SEPARATOR(*lhs))
        lhs++;
    *next = lhs;
    return true;
}

// 获取 dir 目录下的 name 目录 所在的 dentry_t 和 buffer_t
static buffer_t *find_entry(inode_t **dir, const char *name, char **next, dentry_t **result)
{
    // 保证 dir 是目录
    assert(ISDIR((*dir)->desc->mode));

    // if (match_name(name, "..", next) && (*dir)->nr == 1)
    // {
    //     super_block_t *sb = get_super((*dir)->dev);
    //     inode_t *inode = *dir;
    //     (*dir) = sb->imount;
    //     (*dir)->count++;
    //     iput(inode);
    // }

    // 获取目录所在超级块
    // super_block_t *sb = read_super((*dir)->dev);

    // dir 目录最多子目录数量
    u32 entries = (*dir)->desc->size / sizeof(dentry_t);

    idx_t i = 0;
    idx_t block = 0;
    buffer_t *buf = NULL;
    dentry_t *entry = NULL;
    idx_t nr = EOF;

    for (; i < entries; i++, entry++)
    {
        if (!buf || (u32)entry >= (u32)buf->data + BLOCK_SIZE)
        {
            brelse(buf);
            block = bmap((*dir), i / BLOCK_DENTRIES, false);
            assert(block);

            buf = bread((*dir)->dev, block);
            entry = (dentry_t *)buf->data;
        }
        if (match_name(name, entry->name, next) && entry->nr)
        {
            *result = entry;
            return buf;
        }
    }

    brelse(buf);
    return NULL;
}

// 在 dir 目录中添加 name 目录项
static buffer_t *add_entry(inode_t *dir, const char *name, dentry_t **result)
{
    char *next = NULL;

    buffer_t *buf = find_entry(&dir, name, &next, result);
    if (buf)
    {
        return buf;
    }

    // name 中不能有分隔符
    for (size_t i = 0; i < NAME_LEN && name[i]; i++)
    {
        assert(!IS_SEPARATOR(name[i]));
    }

    // super_block_t *sb = get_super(dir->dev);
    // assert(sb);

    idx_t i = 0;
    idx_t block = 0;
    dentry_t *entry;

    for (; true; i++, entry++)
    {
        if (!buf || (u32)entry >= (u32)buf->data + BLOCK_SIZE)
        {
            brelse(buf);
            block = bmap(dir, i / BLOCK_DENTRIES, true);
            assert(block);

            buf = bread(dir->dev, block);
            entry = (dentry_t *)buf->data;
        }
        if (i * sizeof(dentry_t) >= dir->desc->size)
        {
            entry->nr = 0;
            dir->desc->size = (i + 1) * sizeof(dentry_t);
            dir->buf->dirty = true;
        }
        if (entry->nr)
            continue;

        strncpy(entry->name, name, NAME_LEN);
        buf->dirty = true;
        dir->desc->mtime = time();
        dir->buf->dirty = true;
        *result = entry;
        return buf;
    };
}

// 获取 pathname 对应的父目录 inode
inode_t *named(char *pathname, char **next)
{
    inode_t *inode = NULL;
    task_t *task = running_task();
    char *left = pathname;
    if (IS_SEPARATOR(left[0]))
    {
        inode = task->iroot;
        left++;
    }
    else if (left[0])
        inode = task->ipwd;
    else
        return NULL;

    inode->count++;
    *next = left;

    if (!*left)
    {
        return inode;
    }

    char *right = strrsep(left);
    if (!right || right < left)
    {
        return inode;
    }

    right++;

    *next = left;
    dentry_t *entry = NULL;
    buffer_t *buf = NULL;
    while (true)
    {
        brelse(buf);
        buf = find_entry(&inode, left, next, &entry);
        if (!buf)
            goto failure;

        dev_t dev = inode->dev;
        iput(inode);
        inode = iget(dev, entry->nr);
        if (!ISDIR(inode->desc->mode) || !permission(inode, P_EXEC))
            goto failure;

        if (right == *next)
            goto success;

        left = *next;
    }

success:
    brelse(buf);
    return inode;

failure:
    brelse(buf);
    iput(inode);
    return NULL;
}

// 获取 pathname 对应的 inode
inode_t *namei(char *pathname)
{
    char *next = NULL;
    inode_t *dir = named(pathname, &next);
    if (!dir)
        return NULL;
    if (!(*next))
        return dir;

    char *name = next;
    dentry_t *entry = NULL;
    buffer_t *buf = find_entry(&dir, name, &next, &entry);
    if (!buf)
    {
        iput(dir);
        return NULL;
    }

    inode_t *inode = iget(dir->dev, entry->nr);

    iput(dir);
    brelse(buf);
    return inode;
}

void dir_test()
{
    inode_t *inode = namei("d1/d2/../../hello.txt");
    char *buf = (char *)alloc_kpage(1);
    int i = inode_read(inode, buf, 1024, 0);
    LOGK("content: %s\n", buf);
    memset(buf, 'A', PAGE_SIZE);
    inode_write(inode, buf, PAGE_SIZE, 0);
    memset(buf, 'B', PAGE_SIZE);
    inode_write(inode, buf, PAGE_SIZE, PAGE_SIZE);
}
