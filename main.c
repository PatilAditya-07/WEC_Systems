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

ext2_inode_t read_inode(FILE* fp, ext2_superblock_t *sb, ext2_bgd_t *bgd, uint32_t inode_num) {
    uint32_t block_group = (inode_num - 1) / sb->s_inodes_per_group;
    uint32_t index_in_table = (inode_num - 1) % sb->s_inodes_per_group;
    uint32_t inode_table_offset = bgd[block_group].bg_inode_table * (1024 << sb->s_log_block_size) + index_in_table * sb->s_inode_size;

    if (fseek(fp, inode_table_offset, SEEK_SET) != 0) {
        fprintf(stderr, "File seek error\n");
        exit(1);
    }

    ext2_inode_t inode;
    if (fread(&inode, sizeof(ext2_inode_t), 1, fp) != 1) {
        fprintf(stderr, "File read error\n");
        exit(1);
    }

    return inode;
}

void print_inode(ext2_inode_t *inode, uint32_t inode_num) {
    printf("    Inode %u:  \n", inode_num);
    printf("Mode: 0x%04X (%s)\n", inode->i_mode, (inode->i_mode & 0xF000) == 0x4000 ? "directory" :
           (inode->i_mode & 0xF000) == 0x8000 ? "regular file" : "other");
    printf("Size: %u bytes\n", inode->i_size);
    printf("Links count: %u\n", inode->i_links_count);
    printf("Direct blocks: ");
    for (int i = 0; i < 12; i++) printf("%u ", inode->i_block[i]);
    printf("\n");
    printf("Singly indirect: %u\n", inode->i_block[12]);
    printf("Doubly indirect: %u\n", inode->i_block[13]);
    printf("Triply indirect: %u\n", inode->i_block[14]);
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

    uint32_t group_count = (sb.s_blocks_count + sb.s_blocks_per_group - 1) / sb.s_blocks_per_group;
    ext2_bgd_t *bgd = malloc(group_count * sizeof(ext2_bgd_t));
    if (bgd == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        fclose(fp);
        return 1;
    }

    if (fseek(fp, bgdt_offset, SEEK_SET) != 0) {
        fprintf(stderr, "File seek error\n");
        free(bgd);
        fclose(fp);
        return 1;
    }
    for (int i = 0; i < group_count; i++) {
        if (fread(&bgd[i], sizeof(ext2_bgd_t), 1, fp) != 1) {
            fprintf(stderr, "File read error\n");
            free(bgd);
            fclose(fp);
            return 1;
        }
    }

    ext2_inode_t root_inode = read_inode(fp, &sb, bgd, 2); // '/' has inode 2
    print_inode(&root_inode, 2);

    for (int i = 0; i < group_count; i++) {
        print_bgd(bgd, i);
    }
}
