#include <stdint.h>
#include <stdio.h>
#include "structures.h"
#include "sb_bgdt.h"

void print_superblock(ext2_superblock_t *sb, uint32_t block_size) {
    printf("    Superblock:  \n");
    printf("Magic number: 0x%04X \n", sb->s_magic); // macOS is already little endian
    printf("Inodes count: %u\n", sb->s_inodes_count);
    printf("Blocks count:  %u\n", sb->s_blocks_count);
    printf("Reserved blocks count: %u\n", sb->s_r_blocks_count);
    printf("Free blocks count: %u\n", sb->s_free_blocks_count);
    printf("Free inodes count: %u\n", sb->s_free_inodes_count);
    printf("First data block: %u\n", sb->s_first_data_block);
    printf("Block size: %u bytes (log=%u)\n", block_size, sb->s_log_block_size);
    printf("Blocks per group: %u\n", sb->s_blocks_per_group);
    printf("Inodes per group: %u\n", sb->s_inodes_per_group);
    printf("Inode size: %u bytes\n", sb->s_inode_size);
    printf("First non-reserved inode: %u\n", sb->s_first_ino);
    printf("Volume name: %.16s", sb->s_volume_name[0] == '\0' ? "(none)\n" : sb->s_volume_name); // none in this file
    printf("\n");
}

void print_bgd(ext2_bgd_t *bgd, int group_index) {
    printf("    Block Group %d:  \n", group_index);
    printf("Block bitmap block: %u\n", bgd[group_index].bg_block_bitmap);
    printf("Inode bitmap block: %u\n", bgd[group_index].bg_inode_bitmap);
    printf("Inode table start blk: %u\n", bgd[group_index].bg_inode_table);
    printf("Free blocks: %u\n", bgd[group_index].bg_free_blocks_count);
    printf("Free inodes: %u\n", bgd[group_index].bg_free_inodes_count);
    printf("Used directories: %u\n", bgd[group_index].bg_used_dirs_count);
    printf("\n");
}
