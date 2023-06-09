#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "ext2.h"
#include "disk.h"

void get_file_block(FILE* vdisk, const Inode* f_inode, uint32_t f_blk_ix, char* blk_buf) {
    uint32_t blk_ix;
    uint32_t ix_pre_blk;
    uint32_t lim1, lim2;

    ix_pre_blk = DEFAULT_BLK_SIZE / sizeof(uint32_t);

    lim1 = 12 + ix_pre_blk;
    lim2 = 12 + ix_pre_blk + ix_pre_blk * ix_pre_blk;
    lim2 = 12 + ix_pre_blk + ix_pre_blk * ix_pre_blk + ix_pre_blk * ix_pre_blk * ix_pre_blk;

    if (f_blk_ix < 12) {
        blk_ix = f_inode->i_block[f_blk_ix];
    }
    else if (f_blk_ix < lim1) {
        blk_ix = f_inode->i_block[12];
        read_block(vdisk, blk_buf, DEFAULT_BLK_SIZE, blk_ix);

        blk_ix = blk_buf[f_blk_ix - 12];
    }
    else if (f_blk_ix < lim2) {
        blk_ix = f_inode->i_block[13];
        read_block(vdisk, blk_buf, DEFAULT_BLK_SIZE, blk_ix);

        blk_ix = blk_buf[(f_blk_ix - lim1 - 12) / ix_pre_blk];
        read_block(vdisk, blk_buf, DEFAULT_BLK_SIZE, blk_ix);

        blk_ix = blk_buf[(f_blk_ix - lim1 - 12) % ix_pre_blk];
    }
    else {
        blk_ix = f_inode->i_block[14];
        read_block(vdisk, blk_buf, DEFAULT_BLK_SIZE, blk_ix);

        blk_ix = blk_buf[(f_blk_ix - lim2 - lim1 - 12) / (ix_pre_blk * ix_pre_blk)];
        read_block(vdisk, blk_buf, DEFAULT_BLK_SIZE, blk_ix);

        blk_ix = blk_buf[((f_blk_ix - lim2 - lim1 - 12) % (ix_pre_blk * ix_pre_blk)) / ix_pre_blk];
        read_block(vdisk, blk_buf, DEFAULT_BLK_SIZE, blk_ix);

        blk_ix = blk_buf[((f_blk_ix - lim2 - lim1 - 12) % (ix_pre_blk * ix_pre_blk)) % ix_pre_blk];
    }

    read_block(vdisk, blk_buf, DEFAULT_BLK_SIZE, blk_ix);
}

void read_file(FILE* vdisk, const Inode* f_inode, char* f_buf, uint64_t f_size) {
    uint32_t f_blks;                 // 文件块数
    uint32_t f_blk_ix;               // 文件内块索引
    char blk_buf[DEFAULT_BLK_SIZE];  // 通用块缓冲区

    f_blks = f_size / DEFAULT_BLK_SIZE;

    f_blk_ix = 0;

    while (f_blks && f_blk_ix < f_blks - 1) {
        get_file_block(vdisk, f_inode, f_blk_ix, blk_buf);
        memcpy(f_buf + f_blk_ix * DEFAULT_BLK_SIZE, blk_buf, DEFAULT_BLK_SIZE);

        f_blk_ix++;
    }

    get_file_block(vdisk, f_inode, f_blk_ix, blk_buf);
    memcpy(f_buf + f_blk_ix * DEFAULT_BLK_SIZE, blk_buf, f_size - DEFAULT_BLK_SIZE * f_blk_ix);
}

void put_file_block(FILE* vdisk, const Inode* f_inode, uint32_t f_blk_ix, char* blk_buf) {
    uint32_t blk_ix;
    uint32_t ix_pre_blk;
    uint32_t lim1, lim2;

    ix_pre_blk = DEFAULT_BLK_SIZE / sizeof(uint32_t);

    lim1 = 12 + ix_pre_blk;
    lim2 = 12 + ix_pre_blk + ix_pre_blk * ix_pre_blk;
    lim2 = 12 + ix_pre_blk + ix_pre_blk * ix_pre_blk + ix_pre_blk * ix_pre_blk;

    if (f_blk_ix < 12) {
        blk_ix = f_inode->i_block[f_blk_ix];
    }
    else if (f_blk_ix < lim1) {
        blk_ix = f_inode->i_block[12];
        read_block(vdisk, blk_buf, DEFAULT_BLK_SIZE, blk_ix);

        blk_ix = blk_buf[f_blk_ix - 12];
    }
    else if (f_blk_ix < lim2) {
        blk_ix = f_inode->i_block[13];
        read_block(vdisk, blk_buf, DEFAULT_BLK_SIZE, blk_ix);

        blk_ix = blk_buf[(f_blk_ix - lim1 - 12) / ix_pre_blk];
        read_block(vdisk, blk_buf, DEFAULT_BLK_SIZE, blk_ix);

        blk_ix = blk_buf[(f_blk_ix - lim1 - 12) % ix_pre_blk];
    }
    else {
        blk_ix = f_inode->i_block[14];
        read_block(vdisk, blk_buf, DEFAULT_BLK_SIZE, blk_ix);

        blk_ix = blk_buf[(f_blk_ix - lim2 - lim1 - 12) / (ix_pre_blk * ix_pre_blk)];
        read_block(vdisk, blk_buf, DEFAULT_BLK_SIZE, blk_ix);

        blk_ix = blk_buf[((f_blk_ix - lim2 - lim1 - 12) % (ix_pre_blk * ix_pre_blk)) / ix_pre_blk];
        read_block(vdisk, blk_buf, DEFAULT_BLK_SIZE, blk_ix);

        blk_ix = blk_buf[((f_blk_ix - lim2 - lim1 - 12) % (ix_pre_blk * ix_pre_blk)) % ix_pre_blk];
    }

    write_block(vdisk, blk_buf, DEFAULT_BLK_SIZE, blk_ix);
}

void write_file(FILE* vdisk, const Inode* f_inode, char* f_buf, uint64_t f_size) {
    uint32_t f_blks;                 // 文件块数
    uint32_t f_blk_ix;               // 文件内块索引
    char blk_buf[DEFAULT_BLK_SIZE];  // 通用块缓冲区

    f_blks = f_size  / DEFAULT_BLK_SIZE;

    f_blk_ix = 0;

    while (f_blks && f_blk_ix < f_blks - 1) {
        memcpy(blk_buf, f_buf + f_blk_ix * DEFAULT_BLK_SIZE, DEFAULT_BLK_SIZE);
        put_file_block(vdisk, f_inode, f_blk_ix, blk_buf);

        f_blk_ix++;
    }

    memcpy(blk_buf, f_buf + f_blk_ix * DEFAULT_BLK_SIZE, f_size - DEFAULT_BLK_SIZE * f_blk_ix);
    put_file_block(vdisk, f_inode, f_blk_ix, blk_buf);
}