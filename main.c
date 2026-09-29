#include <stdio.h>
#include <stdlib.h>

#include "structures.h"

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
    printf("Volume name: %s\n", sb->s_volume_name);
    printf("\n");
}

void print_bgd(ext2_bgd_t *bgd, int group_index) {
    printf("    Block Group %d:  \n", group_index);
    printf("Block bitmap block: %u\n", bgd->bg_block_bitmap);
    printf("Inode bitmap block: %u\n", bgd->bg_inode_bitmap);
    printf("Inode table start blk: %u\n", bgd->bg_inode_table);
    printf("Free blocks: %u\n", bgd->bg_free_blocks_count);
    printf("Free inodes: %u\n", bgd->bg_free_inodes_count);
    printf("Used directories: %u\n", bgd->bg_used_dirs_count);
    printf("\n");
}
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Invalid args, usage: %s <.img>\n", argv[0]);
        return 1;
    }

    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "File not found\n");
        return 1;
    }
    if (fseek(fp, 1024, SEEK_SET) != 0) {
        fprintf(stderr, "File seek error\n");
        fclose(fp);
        return 1;
    }
    ext2_superblock_t sb;
    if (fread(&sb, 1024, 1, fp) != 1) {
        fprintf(stderr, "File read error\n");
        fclose(fp);
        return 1;
    }
    uint32_t block_size = 1024 << sb.s_log_block_size;
    print_superblock(&sb, block_size);

    uint32_t bgdt_block = sb.s_first_data_block + 1;
    uint32_t bgdt_offset = bgdt_block * block_size;

    ext2_bgd_t bgd;
    uint32_t group_count = (sb.s_blocks_count + sb.s_blocks_per_group - 1) / sb.s_blocks_per_group;

    for (int i = 0; i < group_count; i++) {
        if (fseek(fp, bgdt_offset + (i * 32), SEEK_SET) != 0) {
            fprintf(stderr, "File seek error\n");
            fclose(fp);
            return 1;
        }
        if (fread(&bgd, sizeof(ext2_bgd_t), 1, fp) != 1) {
            fprintf(stderr, "File read error\n");
            fclose(fp);
            return 1;
        }
        print_bgd(&bgd, i);
    }
}
