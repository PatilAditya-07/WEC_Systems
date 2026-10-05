//
// Created by Aditya Patil on 05/10/26.
//

#include "read_write.h"

uint64_t get_inode_offset(ext2_bgd_t *bgd, ext2_superblock_t *sb, uint32_t inode_num) {
    uint32_t block_size = 1024 << sb->s_log_block_size;
    uint32_t group = (inode_num - 1) / sb->s_inodes_per_group;
    uint32_t index_in_group = (inode_num - 1) % sb->s_inodes_per_group;

    return (uint64_t)bgd[group].bg_inode_table * block_size + (uint64_t)index_in_group * sb->s_inode_size;
}

int write_inode(FILE *fp, ext2_superblock_t *sb, ext2_bgd_t *bgd, uint32_t inode_num, ext2_inode_t *inode) {
    uint64_t inode_offset = get_inode_offset(bgd, sb, inode_num);
    if (fseek(fp, inode_offset, SEEK_SET) != 0) {
        fprintf(stderr, "inode seek failed");
        return 0;
    }
    if (fwrite(inode, sizeof(ext2_inode_t), 1, fp) != 1) {
        fprintf(stderr, "write inode failed");
        return 0;
    }
    return 1;
}

int write_superblock(FILE *fp, ext2_superblock_t *sb) {
    if (fseek(fp, 1024, SEEK_SET) != 1 || fwrite(sb, 1024, 1, fp) != 1) {
        fprintf(stderr, "write superblock failed");
        return 0;
    }
    return 1;
}

int write_bgd(FILE *fp, ext2_superblock_t *sb, ext2_bgd_t *bgd, uint32_t group) {
    uint32_t block_size = 1024 << sb->s_log_block_size;
    uint32_t bgdt_block = sb->s_first_data_block + 1; // after sb
    uint64_t bgdt_offset = bgdt_block * block_size;
    uint64_t bgd_offset = (uint64_t)bgdt_offset + group * sizeof(ext2_bgd_t);

    if (fseek(fp, bgd_offset, SEEK_SET) != 0) {
        fprintf(stderr, "bgd seek failed");
        return 0;
    }
    if (fwrite(&bgd[group], sizeof(ext2_bgd_t), 1, fp) != 1) {
        fprintf(stderr, "write bgd failed");
        return 0;
    }
    return 1;
}

int write_block(FILE *fp, uint32_t block_num, ext2_superblock_t *sb, uint8_t *buffer) {
    uint32_t block_size = 1024 << sb->s_log_block_size;
    uint64_t offset = (uint64_t)block_num * (uint64_t)block_size;
    if (fseek(fp, offset, SEEK_SET) != 0) {
        fprintf(stderr, "block seek failed");
        return 0;
    }
    return fwrite(buffer, block_size, 1, fp) == 1;
}

int read_block(FILE *fp, uint32_t block_num, ext2_superblock_t *sb, uint8_t *buffer) {
    uint32_t block_size = 1024 << sb->s_log_block_size;
    uint64_t offset = (uint64_t)block_num * (uint64_t)block_size;
    if (fseek(fp, offset, SEEK_SET) != 0) {
        fprintf(stderr, "block seek failed");
        return 0;
    }
    return fread(buffer, block_size, 1, fp) == 1;
}

int zero_block(FILE *fp, uint32_t block_num, ext2_superblock_t *sb) {
    uint32_t block_size = 1024 << sb->s_log_block_size;
    uint8_t *zero_buffer = calloc(1, block_size);
    if (zero_buffer == NULL) {
        fprintf(stderr, "calloc failed");
        return 0;
    }
    int result = write_block(fp, block_num, sb, zero_buffer);
    free(zero_buffer);
    return result;
}

uint32_t allocate_block(FILE *fp, ext2_superblock_t *sb, ext2_bgd_t *bgd) {
    uint32_t block_size = 1024 << sb->s_log_block_size;
    uint32_t group_count = (sb->s_blocks_count + sb->s_blocks_per_group - 1) / sb->s_blocks_per_group;
    uint8_t *bitmap = malloc(block_size);
    if (bitmap == NULL) {
        fprintf(stderr, "malloc failed");
        return 0;
    }

    for (uint32_t group = 0; group < group_count; group++) {
        uint32_t first_block = sb->s_first_data_block + group * sb->s_blocks_per_group;
        if (first_block >= sb->s_blocks_count) continue;

        uint32_t blocks_in_group = sb->s_blocks_count - first_block;
        if (blocks_in_group > sb->s_blocks_per_group) blocks_in_group = sb->s_blocks_per_group;

        uint32_t bitmap_block = bgd[group].bg_block_bitmap;
        if (!read_block(fp, bitmap_block, sb, bitmap)) {
            free(bitmap);
            return 0;
        }

        for (uint32_t bit = 0; bit < blocks_in_group; bit++) {
            uint32_t byte_index = bit / 8;
            uint32_t bit_index = bit % 8;

            if ((bitmap[byte_index] & (1 << bit_index))== 0){
                uint32_t new_block = first_block + bit;
                bitmap[byte_index] |= (1 << bit_index);

                if (!write_block(fp, bitmap_block, sb, bitmap)) {
                    free(bitmap);
                    return 0;
                }
                if (!write_superblock(fp, sb)) {
                    free(bitmap);
                    return 0;
                }

                if (!zero_block(fp, new_block, sb)) {
                    free(bitmap);
                    return 0;
                }

                free(bitmap);
                return new_block;
            }
        }
    }
    free(bitmap);
    return 0;
}
