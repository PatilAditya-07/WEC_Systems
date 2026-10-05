#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "structures.h"
#include "sb_bgdt.h"
#include "directory.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <.img> -[1|2|3|4]\n", argv[0]);
        fprintf(stderr, "  -1  Task 1: Print Superblock and BGDT\n");
        fprintf(stderr, "  -2  Task 2: Print Directory Tree\n");
        return 1;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("Failed to open disk image");
        return 1;
    }

    // read superblock
    if (fseek(fp, 1024, SEEK_SET) != 0) {
        perror("Seek error");
        fclose(fp);
        return 1;
    }
    ext2_superblock_t sb;
    if (fread(&sb, 1024, 1, fp) != 1) {
        perror("Read error");
        fclose(fp);
        return 1;
    }
    uint32_t block_size = 1024 << sb.s_log_block_size;

    // read bgdt
    uint32_t bgdt_block = sb.s_first_data_block + 1;
    uint32_t bgdt_offset = bgdt_block * block_size;
    uint32_t group_count = (sb.s_blocks_count + sb.s_blocks_per_group - 1) / sb.s_blocks_per_group;

    ext2_bgd_t *bgd = malloc(group_count * sizeof(ext2_bgd_t));
    if (!bgd) {
        fprintf(stderr, "Allocation error\n");
        fclose(fp);
        return 1;
    }

    if (fseek(fp, bgdt_offset, SEEK_SET) != 0 ||
        fread(bgd, sizeof(ext2_bgd_t), group_count, fp) != group_count) {
        perror("BGDT read error");
        free(bgd);
        fclose(fp);
        return 1;
    }

    int opt;
    while ((opt = getopt(argc - 1, argv + 1, "123:4:")) != -1) {
        switch (opt) {
            case '1': // Task 1
                print_superblock(&sb, block_size);
                for (uint32_t i = 0; i < group_count; i++) {
                    print_bgd(bgd, i);
                }
                break;
            case '2': // Task 2
                printf("/\n");
                traverse_directory(fp, 2, &sb, bgd, 0);
                break;
            default:
                fprintf(stderr, "Invalid option\n");
                break;
        }
    }

    free(bgd);
    fclose(fp);
    return 0;
}