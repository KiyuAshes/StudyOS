#include <stdio.h>
#include <stdint.h>

uint64_t read_block(FILE* vdisk, char* buf, uint64_t blk_size, uint32_t blk_ix);
uint64_t write_block(FILE* vdisk, char* buf, uint32_t blk_size, uint32_t blk_ix);