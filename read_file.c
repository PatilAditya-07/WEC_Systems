//
// Created by Aditya Patil on 05/10/26.
//

#include "read_file.h"
#include "structures.h"
#include <stdio.h>
#include <stdint.h>

uint32_t get_actual_block(FILE* fp, ext2_inode_t *inode, ext2_superblock_t *sb, uint32_t block_num) {
    uint32_t block_size = 1024 << sb->s_log_block_size;
    uint32_t ptrs_per_block = block_size / 4;
    if (block_num < 12) {
        return inode->i_block[block_num];
    }
    block_num -= 12;
    uint32_t *ptrs = malloc(block_size);
    if (!ptrs) {
        fprintf(stderr, "Allocation error");
        exit(1);
    }

    if (block_num < ptrs_per_block) { // singly indirect
        if (fseek(fp, inode->i_block[12] * block_size, SEEK_SET) != 0) {
            fprintf(stderr, "seek error");
            exit(1);
        }
        if (fread(ptrs, block_size, 1, fp) != 1) {
            fprintf(stderr, "read error");
            exit(1);
        }
        uint32_t block = ptrs[block_num];
        free(ptrs);
        return block;
    }
    // doubly indirect only
    block_num -= ptrs_per_block;
    if (fseek(fp, inode->i_block[13] * block_size, SEEK_SET) != 0) {
        fprintf(stderr, "seek error");
        exit(1);
    }
    uint32_t *level2 = malloc(block_size), *level1 = malloc(block_size);
    if (!level2 || !level1) {
        fprintf(stderr, "Allocation error");
        exit(1);
    }
    if (fread(level2, block_size, 1, fp) != 1) {
        fprintf(stderr, "read error");
        exit(1);
    }
    uint32_t level2_id = block_num / ptrs_per_block;
    if (fseek(fp, level2[level2_id] * block_size, SEEK_SET) != 0) {
        fprintf(stderr, "seek error");
        exit(1);
    }
    if (fread(level1, block_size, 1, fp) != 1) {
        fprintf(stderr, "read error");
        exit(1);
    }
    uint32_t level1_id = block_num % ptrs_per_block;
    uint32_t block = level1[level1_id];
    free(level1);
    free(level2);
    return block;
}

void read_file(FILE* fp, ext2_inode_t *inode, ext2_superblock_t *sb, FILE* op) {
    uint32_t block_size = 1024 << sb->s_log_block_size;
    uint32_t file_size = inode->i_size;
    uint8_t *buffer = malloc(block_size);
    if (!buffer) {
        fprintf(stderr, "Allocation error");
        exit(1);
    }

    uint32_t num_blocks = (file_size + block_size - 1) / block_size;
    uint32_t remaining = file_size;

    for (uint32_t block = 0; block < num_blocks; block++) {
        uint32_t actual_block = get_actual_block(fp, inode, sb, block);
        if (actual_block == 0) {
            fprintf(stderr, "Invalid block");
            exit(1);
        }
        if (fseek(fp, actual_block * block_size, SEEK_SET) != 0) {
            fprintf(stderr, "seek error");
            exit(1);
        }
        if (fread(buffer, block_size, 1, fp) != 1) {
            fprintf(stderr, "read error");
            exit(1);
        }
        uint32_t bytes_write = remaining < block_size ? remaining : block_size;
        fwrite(buffer, 1, bytes_write, op); // file contents written to file op
        remaining -= bytes_write;
    }
    free(buffer);
}