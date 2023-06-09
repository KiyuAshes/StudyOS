#include <stdio.h>

#include "ext2.h"

void read_file(FILE* vdisk, const Inode* f_inode, char* f_buf, uint64_t f_size);
void get_file_block(FILE* vdisk, const Inode* f_inode, uint32_t f_blk_ix, char* blk_buf);

void write_file(FILE* vdisk, const Inode* f_inode, char* f_buf, uint64_t f_size);
void put_file_block(FILE* vdisk, const Inode* f_inode, uint32_t f_blk_ix, char* blk_buf);
