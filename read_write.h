//
// Created by Aditya Patil on 05/10/26.
//

#ifndef WEC_SYSTEMS_OS_MODIFY_H
#define WEC_SYSTEMS_OS_MODIFY_H
#include "structures.h"
#include <stdio.h>
#include <stdint.h>

int write_inode(FILE *fp, ext2_superblock_t *sb, ext2_bgd_t *bgd, uint32_t inode_num, ext2_inode_t *inode);
uint64_t get_inode_offset(ext2_bgd_t *bgd, ext2_superblock_t *sb, uint32_t inode_num);
int write_superblock(FILE *fp, ext2_superblock_t *sb);
int write_bgd(FILE *fp, ext2_superblock_t *sb, ext2_bgd_t *bgd, uint32_t group);
int write_block(FILE *fp, uint32_t block_num, ext2_superblock_t *sb, uint8_t *buffer);
int read_block(FILE *fp, uint32_t block_num, ext2_superblock_t *sb, uint8_t *buffer);
int zero_block(FILE *fp, uint32_t block_num, ext2_superblock_t *sb);
uint32_t allocate_block(FILE *fp, ext2_superblock_t *sb, ext2_bgd_t *bgd);

#endif //WEC_SYSTEMS_OS_MODIFY_H
