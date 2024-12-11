#include <linux/fs.h>
#include <linux/pagemap.h>
#include <linux/slab.h>
#include "osfs.h"

/**
 * Struct: osfs_super_ops
 * Description: Defines the superblock operations for the osfs filesystem.
 */
const struct super_operations osfs_super_ops = {
    .statfs = simple_statfs,            // Provides filesystem statistics
    .drop_inode = generic_delete_inode, // Generic inode deletion
    .destroy_inode = osfs_destroy_inode,
    .put_super = osfs_put_super
};

void osfs_destroy_inode(struct inode *inode)
{
    if (inode->i_private) {
        inode->i_private = NULL;
    }
}

void osfs_put_super(struct super_block *sb)
{
    struct osfs_sb_info *sb_info = sb->s_fs_info;

    if (sb_info) {
        // Free block bitmap
        if (sb_info->block_bitmap)
            kfree(sb_info->block_bitmap);
        // Free data blocks
        if (sb_info->data_blocks)
            kfree(sb_info->data_blocks);
    
        // Free superblock info structure
        kfree(sb_info);
        sb->s_fs_info = NULL;
    }
}

struct inode *osfs_get_root_inode(struct super_block *sb)
{
    struct inode *inode;
    struct osfs_inode *osfs_inode;

    inode = new_inode(sb);
    if (!inode)
        return NULL;

    inode->i_ino = ROOT_INODE_NUMBER;
    inode_init_owner(&nop_mnt_idmap, inode, NULL, S_IFDIR | 0755);
    inode->i_op = &osfs_dir_inode_operations;
    inode->i_fop = &osfs_dir_operations;
    inode->i_sb = sb;

    // Allocate and initialize osfs_inode
    osfs_inode = kzalloc(sizeof(struct osfs_inode), GFP_KERNEL);
    if (!osfs_inode) {
        iput(inode);
        return NULL;
    }

    // Initialize osfs_inode fields
    osfs_inode->i_size = 0;
    osfs_inode->i_extents_count = 0;
    memset(osfs_inode->i_extents, 0, sizeof(osfs_inode->i_extents));

    inode->i_private = osfs_inode;

    return inode;
}

/**
 * Function: osfs_fill_super
 * Description: Initializes the superblock with filesystem-specific information during mount.
 * Inputs:
 *   - sb: The superblock to be filled.
 *   - data: Data passed during mount.
 *   - silent: If non-zero, suppress certain error messages.
 * Returns:
 *   - 0 on successful initialization.
 *   - A negative error code on failure.
 */
int osfs_fill_super(struct super_block *sb, void *data, int silent)
{
    struct osfs_sb_info *sb_info;
    struct inode *root_inode;
    int ret;

    // Allocate memory for the superblock info
    sb_info = kzalloc(sizeof(struct osfs_sb_info), GFP_KERNEL);
    if (!sb_info)
        return -ENOMEM;

    // Initialize total_blocks (set this to the number of data blocks in your filesystem)
    sb_info->block_count = DATA_BLOCK_COUNT;

    // Allocate and initialize the block bitmap
    sb_info->block_bitmap = kzalloc(BITS_TO_LONGS(sb_info->block_count) * sizeof(long), GFP_KERNEL);
    if (!sb_info->block_bitmap) {
        kfree(sb_info);
        return -ENOMEM;
    }


    // Initialize data_blocks (pointer to the start of data block region)
    sb_info->data_blocks = kzalloc(sb_info->block_count * BLOCK_SIZE, GFP_KERNEL);
    // Assign sb_info to the superblock's s_fs_info
    sb->s_fs_info = sb_info;

    // Set superblock operations
    sb->s_op = &osfs_super_ops;

    // Create and initialize the root inode
    root_inode = osfs_get_root_inode(sb);
    if (!root_inode) {
        pr_err("osfs_fill_super: Failed to create root inode\n");
        ret = -ENOMEM;
        goto cleanup;
    }

    // Set up the root directory
    sb->s_root = d_make_root(root_inode);
    if (!sb->s_root) {
        pr_err("osfs_fill_super: Failed to create root dentry\n");
        ret = -ENOMEM;
        goto cleanup;
    }

    return 0;

cleanup:
    // Free allocated resources on failure
    kfree(sb_info->block_bitmap);
    kfree(sb_info);
    return ret;
}