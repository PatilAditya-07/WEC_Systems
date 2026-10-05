//
// Created by Aditya Patil on 05/10/26.
//

#ifndef WEC_SYSTEMS_OS_SB_BGDT_H
#define WEC_SYSTEMS_OS_SB_BGDT_H
#include <stdint.h>
#include "structures.h"

void print_superblock(ext2_superblock_t *sb, uint32_t block_size);
void print_bgd(ext2_bgd_t *bgd, int group_index);

#endif //WEC_SYSTEMS_OS_SB_BGDT_H
