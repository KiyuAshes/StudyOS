#include <stdio.h>
#include <stdint.h>

uint64_t read_block(FILE* vdisk, char* buf, uint32_t blk_size, uint32_t blk_ix) {
    uint64_t offset = blk_size * blk_ix;  // 计算块在虚拟硬盘文件中的偏移

    fseek(vdisk, offset, SEEK_SET);

    uint32_t ret_size = fread(buf, sizeof *buf, blk_size, vdisk);

    if (ret_size != blk_size && !feof(vdisk)) {
        return 0;
    }

    return ret_size;
}

uint64_t write_block(FILE* vdisk, char* buf, uint32_t blk_size, uint32_t blk_ix) {
    uint64_t offset = blk_size * blk_ix;  // 计算块在虚拟硬盘文件中的偏移

    fseek(vdisk, offset, SEEK_SET);

    uint32_t ret_size = fwrite(buf, sizeof *buf, blk_size, vdisk);

    if (ret_size != blk_size && !feof(vdisk)) {
        return 0;
    }

    return ret_size;
}