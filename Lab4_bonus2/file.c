#include <linux/fs.h>
#include <linux/uaccess.h>
#include "osfs.h"

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
    void *data_block;
    ssize_t bytes_read;

    // If the file has not been allocated a data block, it indicates the file is empty
    if (osfs_inode->extent[0].block_count == 0)
        return 0;

    if (*ppos >= osfs_inode->i_size)
        return 0;

    if (*ppos + len > osfs_inode->i_size)
        len = osfs_inode->i_size - *ppos;

    loff_t idx = *ppos / (MAX_CON_BLOCKS*BLOCK_SIZE);
    loff_t offset =*ppos % (MAX_CON_BLOCKS*BLOCK_SIZE);

    data_block = sb_info->data_blocks + osfs_inode->extent[idx].start_block * BLOCK_SIZE + offset;
    if (copy_to_user(buf, data_block, len))
        return -EFAULT;

    *ppos += len;
    bytes_read = len;

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
    //Step1: Retrieve the inode and filesystem information
    struct inode *inode = file_inode(filp);
    struct osfs_inode *osfs_inode = inode->i_private;
    struct osfs_sb_info *sb_info = inode->i_sb->s_fs_info;
    void *data_block;
    ssize_t bytes_written;
    int ret;
    loff_t idx = *ppos / (MAX_CON_BLOCKS*BLOCK_SIZE);
    loff_t offset =*ppos % (MAX_CON_BLOCKS*BLOCK_SIZE);

    // Step2: Check if a data block has been allocated; if not, allocate one
    if (osfs_inode->extent_count < MAX_EXTENT && osfs_inode->extent[idx].block_count == 0) {
        ret = osfs_alloc_data_block(sb_info, &osfs_inode->extent[idx].start_block);
        if (ret) {
            pr_err("osfs_write: Failed to allocate data block\n");
            return ret;
        }
        osfs_inode->extent[idx].block_count =MAX_CON_BLOCKS;
        osfs_inode->extent_count ++;
    }

    // Step3: Limit the write length to fit within one data block
    if (*ppos  >=BLOCK_SIZE*MAX_CON_BLOCKS*MAX_EXTENT){return 0;}
    if (*ppos + len > BLOCK_SIZE*MAX_CON_BLOCKS*MAX_EXTENT) {
        len = BLOCK_SIZE*MAX_CON_BLOCKS*MAX_EXTENT - *ppos;
        pr_warn("osfs_write: Adjusting write length to %zu\n", len);
    }

    // Step4: Write data from user space to the data block
    data_block = sb_info->data_blocks + osfs_inode->extent[idx].start_block * BLOCK_SIZE + offset;
    if (copy_from_user(data_block, buf, len)){
        pr_err("osfs_write: Failed to copy data from user space\n");
        return -EFAULT;
    }

    // Step5: Update inode & osfs_inode attribute
    bytes_written = len;
    *ppos+=len;

    if (*ppos > osfs_inode->i_size){
        osfs_inode->extent[idx].file_offset=offset;
        osfs_inode->i_size = *ppos;
        inode->i_size = *ppos;
    }
    inode->__i_mtime = current_time(inode);
    inode->__i_ctime = current_time(inode);
    mark_inode_dirty(inode);

    // Step6: Return the number of bytes written
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
