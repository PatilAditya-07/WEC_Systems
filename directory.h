//
// Created by Aditya Patil on 05/10/26.
//

#ifndef WEC_SYSTEMS_OS_DIRECTORY_H
#define WEC_SYSTEMS_OS_DIRECTORY_H
#include "structures.h"
#include <stdint.h>
#include <stdio.h>

ext2_inode_t read_inode(FILE* fp, ext2_superblock_t *sb, ext2_bgd_t *bgd, uint32_t inode_num);
void print_inode(ext2_inode_t *inode, uint32_t inode_num);
void traverse_directory(FILE* fp, uint32_t inode_num, ext2_superblock_t *sb, ext2_bgd_t *bgd, uint32_t depth);

#endif //WEC_SYSTEMS_OS_DIRECTORY_H
