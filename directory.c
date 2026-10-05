//
// Created by Aditya Patil on 05/10/26.
//

#include "directory.h"
#include <stdio.h>
#include <string.h>

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

void read_direct_block(FILE* fp, uint32_t block_num,
                               ext2_superblock_t *sb, ext2_bgd_t *bgd, uint32_t depth) {
    uint32_t block_size = 1024 << sb->s_log_block_size;
    uint8_t *buffer = malloc(block_size);
    if (fseek(fp, block_num * block_size, SEEK_SET) != 0) {
        fprintf(stderr, "File seek error\n");
        free(buffer);
        exit(1);
    }
    if (fread(buffer, block_size, 1, fp) != 1) {
        fprintf(stderr, "File read error\n");
        free(buffer);
        exit(1);
    }

    uint32_t offset = 0;
    while (offset < block_size) {
        ext2_dir_entry_t *dir = (ext2_dir_entry_t *)(buffer + offset);
        if (dir->rec_len == 0 || (offset + dir->rec_len) > block_size) break;

        if (dir->inode != 0) {
            char filename[dir->name_len + 1];
            memcpy(filename, dir->name, dir->name_len);
            filename[dir->name_len] = '\0';

            if (strcmp(filename, ".") != 0 && strcmp(filename, "..") != 0) {
                for (uint32_t j = 0; j < depth; j++) printf("│   ");
                if (dir->file_type == 2) {
                    printf("├──  %s/\n", filename);
                    traverse_directory(fp, dir->inode, sb, bgd, depth + 1);
                } else {
                    printf("├──  %s\n", filename);
                }
            }
        }
        offset += dir->rec_len;
    }
    free(buffer);
}

void read_indirect_block(FILE* fp, uint32_t indirect_block_num, ext2_superblock_t *sb,
                            ext2_bgd_t *bgd, uint32_t depth, uint32_t levels_left, uint32_t *rem_direct) {
    uint32_t block_size = 1024 << sb->s_log_block_size;
    if (indirect_block_num == 0 || rem_direct == 0) return;

    uint32_t ptrs_per_block = block_size / 4; // 32 bit block
    uint32_t *ptrs = malloc(block_size);
    if (fseek(fp, indirect_block_num * block_size, SEEK_SET) != 0) {
        fprintf(stderr, "File seek error\n");
        exit(1);
    }
    if (fread(ptrs, block_size, 1, fp) != 1) {
        fprintf(stderr, "File read error\n");
        exit(1);
    }
    for (uint32_t i = 0; i < ptrs_per_block; i++) {
        if (*rem_direct == 0) break;
        if (ptrs[i] == 0) continue;

        if (levels_left == 1) {
            read_direct_block(fp, ptrs[i], sb, bgd, depth);
            (*rem_direct)--;
        }
        else {
            read_indirect_block(fp, ptrs[i], sb, bgd, depth, levels_left - 1, rem_direct);
        }
    }
    free(ptrs);
}

void read_dir_entry(FILE* fp, ext2_superblock_t *sb, ext2_bgd_t *bgd, ext2_inode_t *inode, uint32_t depth) {
    uint32_t block_size = 1024 << sb->s_log_block_size;
    uint32_t rem_direct = (inode->i_size + block_size - 1) / block_size; // total blocks
    for (uint32_t i = 0; i < 12 && rem_direct > 0; i++) {
        if (inode->i_block[i] == 0) continue;
        read_direct_block(fp, inode->i_block[i], sb, bgd, depth);
        rem_direct--;
    }
    if (rem_direct > 0 && inode->i_block[12] != 0) { // singly indirect
        read_indirect_block(fp, inode->i_block[12], sb, bgd, depth, 1, &rem_direct);
    }
    if (rem_direct > 0 && inode->i_block[13] != 0) { // doubly indirect
        read_indirect_block(fp, inode->i_block[13], sb, bgd, depth, 2, &rem_direct);
    }
    if (rem_direct > 0 && inode->i_block[14] != 0) { // triply indirect, not reqd for this image
        read_indirect_block(fp, inode->i_block[14], sb, bgd, depth, 3, &rem_direct);
    }
}

void traverse_directory(FILE* fp, uint32_t inode_num, ext2_superblock_t *sb, ext2_bgd_t *bgd, uint32_t depth) {
    ext2_inode_t inode = read_inode(fp, sb, bgd, inode_num);
    if ((inode.i_mode & 0xF000) == 0x4000) { // check if directory
        read_dir_entry(fp, sb, bgd, &inode, depth);
    }
}


