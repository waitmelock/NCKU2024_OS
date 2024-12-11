#include <linux/fs.h>
#include <linux/uaccess.h>
#include "osfs.h"


int osfs_block_map(struct osfs_inode *osfs_inode, loff_t offset, uint32_t *block_num)
{
    size_t cumulative_size = 0;
    int i;

    for (i = 0; i < osfs_inode->i_extents_count; i++) {
        struct osfs_extent *extent = &osfs_inode->i_extents[i];
        size_t extent_size = extent->block_count * BLOCK_SIZE;

        if (offset < cumulative_size + extent_size) {
            *block_num = extent->start_block + (offset - cumulative_size) / BLOCK_SIZE;
            return 0;
        }

        cumulative_size += extent_size;
    }

    return -ENODATA;
}
/**
 * Function: osfs_read
 * Description: Reads data from a file.
 * Inputs:
 *   - filp: The file pointer representing the file to read from.
 *   - buf: The user-space buffer to copy the data into.
 *   - len: The number of bytes to read.
 *   - ppos: The file position pointer.
 * Returns:
 *   - The number of bytes read on success.
 *   - 0 if the end of the file is reached.
 *   - -EFAULT if copying data to user space fails.
 */
static ssize_t osfs_read(struct file *filp, char __user *buf, size_t len, loff_t *ppos)
{
    struct inode *inode = file_inode(filp);
    struct osfs_inode *osfs_inode = inode->i_private;
    struct osfs_sb_info *sb_info = inode->i_sb->s_fs_info;
    ssize_t bytes_read = 0;
    size_t offset = *ppos;
    int i;

    if (offset >= osfs_inode->i_size)
        return 0;

    if (offset + len > osfs_inode->i_size)
        len = osfs_inode->i_size - offset;

    for (i = 0; i < osfs_inode->i_extents_count && bytes_read < len; i++) {
        struct osfs_extent *extent = &osfs_inode->i_extents[i];
        size_t extent_start = extent->start_block * BLOCK_SIZE;
        size_t extent_size = extent->block_count * BLOCK_SIZE;

        if (offset >= extent_size) {
            offset -= extent_size;
            continue;
        }

        size_t bytes_to_read = min(len - bytes_read, extent_size - offset);
        void *extent_data = sb_info->data_blocks + extent_start + offset;

        if (copy_to_user(buf + bytes_read, extent_data, bytes_to_read))
            return -EFAULT;

        bytes_read += bytes_to_read;
        offset = 0;  // Reset offset after first extent
    }

    *ppos += bytes_read;
    return bytes_read;
}


/**
 * Function: osfs_write
 * Description: Writes data to a file.
 * Inputs:
 *   - filp: The file pointer representing the file to write to.
 *   - buf: The user-space buffer containing the data to write.
 *   - len: The number of bytes to write.
 *   - ppos: The file position pointer.
 * Returns:
 *   - The number of bytes written on success.
 *   - -EFAULT if copying data from user space fails.
 *   - Adjusted length if the write exceeds the block size.
 */
static ssize_t osfs_write(struct file *filp, const char __user *buf, size_t len, loff_t *ppos)
{
    struct inode *inode = file_inode(filp);
    struct osfs_inode *osfs_inode = inode->i_private;
    struct osfs_sb_info *sb_info = inode->i_sb->s_fs_info;
    ssize_t bytes_written = 0;
    size_t offset = *ppos;
    int  i;

    // Allocate new extent if needed
    if (osfs_inode->i_extents_count == 0 || offset >= osfs_inode->i_size) {
        if (osfs_inode->i_extents_count >= MAX_EXTENTS)
            return -ENOSPC;

        struct osfs_extent *new_extent = &osfs_inode->i_extents[osfs_inode->i_extents_count];
        uint32_t blocks_needed = DIV_ROUND_UP(len, BLOCK_SIZE);


        osfs_inode->i_extents_count++;
        osfs_inode->i_size += blocks_needed * BLOCK_SIZE;
        mark_inode_dirty(inode);
    }

    for (i = 0; i < osfs_inode->i_extents_count && bytes_written < len; i++) {
        struct osfs_extent *extent = &osfs_inode->i_extents[i];
        size_t extent_start = extent->start_block * BLOCK_SIZE;
        size_t extent_size = extent->block_count * BLOCK_SIZE;

        if (offset >= extent_size) {
            offset -= extent_size;
            continue;
        }

        size_t bytes_to_write = min(len - bytes_written, extent_size - offset);
        void *extent_data = sb_info->data_blocks + extent_start + offset;

        if (copy_from_user(extent_data, buf + bytes_written, bytes_to_write))
            return -EFAULT;

        bytes_written += bytes_to_write;
        offset = 0;  // Reset offset after first extent
    }

    *ppos += bytes_written;

    // Update inode metadata
    if (*ppos > osfs_inode->i_size) {
        osfs_inode->i_size = *ppos;
        inode->i_size = *ppos;
    }

    inode->__i_mtime = inode->__i_ctime = current_time(inode);
    mark_inode_dirty(inode);

    return bytes_written;
}

/**
 * Struct: osfs_file_operations
 * Description: Defines the file operations for regular files in osfs.
 */
const struct file_operations osfs_file_operations = {
    .open = generic_file_open, // Use generic open or implement osfs_open if needed
    .read = osfs_read,
    .write = osfs_write,
    .llseek = default_llseek,
    // Add other operations as needed
};

/**
 * Struct: osfs_file_inode_operations
 * Description: Defines the inode operations for regular files in osfs.
 * Note: Add additional operations such as getattr as needed.
 */
const struct inode_operations osfs_file_inode_operations = {
    // Add inode operations here, e.g., .getattr = osfs_getattr,
};
