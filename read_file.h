//
// Created by Aditya Patil on 05/10/26.
//

#ifndef WEC_SYSTEMS_OS_READ_FILE_H
#define WEC_SYSTEMS_OS_READ_FILE_H
#include <stdio.h>
#include "structures.h"

uint32_t get_actual_block(FILE* fp, ext2_inode_t *inode, ext2_superblock_t *sb, uint32_t block_num);
void read_file(FILE* fp, ext2_inode_t *inode, ext2_superblock_t *sb, FILE* op);

#endif //WEC_SYSTEMS_OS_READ_FILE_H
