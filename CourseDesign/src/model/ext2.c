#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "ext2.h"
#include "disk.h"
#include "util.h"

void get_inode(FILE* vdisk, const SuperBlock* super_blk, const GroupDesc* gdt, uint32_t inode_ix, Inode* inode_buf) {
    char blk_buf[DEFAULT_BLK_SIZE];  // 通用块缓冲区

    // 计算Inode节点在哪个块组上
    uint32_t blk_g_ix = (inode_ix - 1) / super_blk->inodes_per_group;

    // 计算Inode节点在该块组中的序号
    uint32_t blk_g_inode_ix = (inode_ix - 1) % super_blk->inodes_per_group;

    // 计算Inode节点在哪个块上
    uint32_t blk_ix = gdt[blk_g_ix].bg_inode_table + (blk_g_inode_ix * DEFAULT_INODE_SIZE - 1) / DEFAULT_BLK_SIZE;

    // 计算Inode节点在该块中的序号
    uint32_t blk_inode_ix = blk_g_inode_ix % (DEFAULT_BLK_SIZE / DEFAULT_INODE_SIZE);

    read_block(vdisk, blk_buf, DEFAULT_BLK_SIZE, blk_ix);

    *inode_buf = *(Inode*)(blk_buf + DEFAULT_INODE_SIZE * blk_inode_ix);
}

void put_inode(FILE* vdisk, const SuperBlock* super_blk, const GroupDesc* gdt, uint32_t inode_ix, const Inode* inode_buf) {
    char blk_buf[DEFAULT_BLK_SIZE];  // 通用块缓冲区

    // 计算Inode节点在哪个块组上
    uint32_t blk_g_ix = (inode_ix - 1) / super_blk->inodes_per_group;

    // 计算Inode节点在该块组中的序号
    uint32_t blk_g_inode_ix = (inode_ix - 1) % super_blk->inodes_per_group;

    // 计算Inode节点在哪个块上
    uint32_t blk_ix = gdt[blk_g_ix].bg_inode_table + (blk_g_inode_ix * DEFAULT_INODE_SIZE - 1) / DEFAULT_BLK_SIZE;

    // 计算Inode节点在该块中的序号
    uint32_t blk_inode_ix = blk_g_inode_ix % (DEFAULT_BLK_SIZE / DEFAULT_INODE_SIZE);

    read_block(vdisk, blk_buf, DEFAULT_BLK_SIZE, blk_ix);

    *(Inode*)(blk_buf + DEFAULT_INODE_SIZE * blk_inode_ix) = *inode_buf;

    write_block(vdisk, blk_buf, DEFAULT_BLK_SIZE, blk_ix);
}

uint32_t get_next_free_inode_ix(FILE* vdisk, const SuperBlock* super_blk, const GroupDesc* gdt) {
    uint32_t inode_ix;
    char blk_buf[DEFAULT_BLK_SIZE];  // 通用块缓冲区

    for (uint16_t i = 0; i < super_blk->inodes_count / super_blk->inodes_per_group; i++) {
        // for (uint16_t j = 0; j < super_blk->inodes_per_group / (8 * DEFAULT_BLK_SIZE); j++) {
        uint16_t j = 0;
        do {
            read_block(vdisk, blk_buf, DEFAULT_BLK_SIZE, gdt[i].bg_inode_bitmap);

            for (uint16_t k = 0; k < DEFAULT_BLK_SIZE * 8; k++) {
                inode_ix = i * super_blk->inodes_per_group + j * 8 * DEFAULT_BLK_SIZE + k;

                // 跳过保留Inode
                if (inode_ix < 12) {
                    continue;
                }

                if (!get_bit(blk_buf, inode_ix)) {
                    return inode_ix;
                }
            }
            j++;
        } while (j < super_blk->inodes_per_group / (8 * DEFAULT_BLK_SIZE));
    }
    return 0;
}

uint32_t get_next_free_block_ix(FILE* vdisk, const SuperBlock* super_blk, const GroupDesc* gdt) {
    uint32_t block_ix;
    char blk_buf[DEFAULT_BLK_SIZE];  // 通用块缓冲区

    for (uint16_t i = 0; i < super_blk->inodes_count / super_blk->inodes_per_group; i++) {
        for (uint16_t j = 0; j < super_blk->inodes_per_group / (8 * DEFAULT_BLK_SIZE); j++) {
            read_block(vdisk, blk_buf, DEFAULT_BLK_SIZE, gdt[i].bg_inode_bitmap + j);
            for (uint16_t k = 0; k < DEFAULT_BLK_SIZE * 8; k++) {
                block_ix = i * super_blk->inodes_per_group + j * 8 * DEFAULT_BLK_SIZE + k;
                if (!get_bit(blk_buf, block_ix)) {
                    return block_ix;
                }
            }
        }
    }
    return 0;
}

uint32_t get_free_inode(FILE* vdisk, const SuperBlock* super_blk, const GroupDesc* gdt) {
    uint32_t inode_ix;
    char blk_buf[DEFAULT_BLK_SIZE];  // 通用块缓冲区

    inode_ix = get_next_free_inode_ix(vdisk, super_blk, gdt);

    // 计算Inode状态位在哪个块组上
    uint32_t blk_g_ix = (inode_ix - 1) / super_blk->inodes_per_group;

    // 计算Inode状态位在该块组中的序号
    uint32_t blk_g_inode_ix = (inode_ix) % super_blk->inodes_per_group;

    // 计算Inode状态位在哪个块上
    uint32_t blk_ix = gdt[blk_g_ix].bg_inode_bitmap + (blk_g_inode_ix) / (8 * DEFAULT_BLK_SIZE);

    // 计算Inode状态位在该块中的序号
    uint32_t blk_inode_ix = blk_g_inode_ix % (DEFAULT_BLK_SIZE * 8);

    read_block(vdisk, blk_buf, DEFAULT_BLK_SIZE, blk_ix);

    set_bit(blk_buf, blk_inode_ix);

    write_block(vdisk, blk_buf, DEFAULT_BLK_SIZE, blk_ix);

    return inode_ix;
}

// uint32_t get_free_block() {}

void release_inode(FILE* vdisk, const SuperBlock* super_blk, const GroupDesc* gdt, uint32_t inode_ix) {
    char blk_buf[DEFAULT_BLK_SIZE];  // 通用块缓冲区

    inode_ix = get_next_free_inode_ix(vdisk, super_blk, gdt);

    // 计算Inode状态位在哪个块组上
    uint32_t blk_g_ix = (inode_ix - 1) / super_blk->inodes_per_group;

    // 计算Inode状态位在该块组中的序号
    uint32_t blk_g_inode_ix = (inode_ix) % super_blk->inodes_per_group;

    // 计算Inode状态位在哪个块上
    uint32_t blk_ix = gdt[blk_g_ix].bg_inode_bitmap + (blk_g_inode_ix) / (8 * DEFAULT_BLK_SIZE);

    // 计算Inode状态位在该块中的序号
    uint32_t blk_inode_ix = blk_g_inode_ix % (DEFAULT_BLK_SIZE * 8);

    read_block(vdisk, blk_buf, DEFAULT_BLK_SIZE, blk_ix);

    reset_bit(blk_buf, blk_inode_ix);

    write_block(vdisk, blk_buf, DEFAULT_BLK_SIZE, blk_ix);

    // 计算Inode节点在哪个块组上
    blk_g_ix = (inode_ix - 1) / super_blk->inodes_per_group;

    // 计算Inode节点在该块组中的序号
    blk_g_inode_ix = (inode_ix - 1) % super_blk->inodes_per_group;

    // 计算Inode节点在哪个块上
    blk_ix = gdt[blk_g_ix].bg_inode_table + (blk_g_inode_ix * DEFAULT_INODE_SIZE - 1) / DEFAULT_BLK_SIZE;

    // 计算Inode节点在该块中的序号
    blk_inode_ix = blk_g_inode_ix % (DEFAULT_BLK_SIZE / DEFAULT_INODE_SIZE);

    read_block(vdisk, blk_buf, DEFAULT_BLK_SIZE, blk_ix);

    memset(blk_buf + DEFAULT_INODE_SIZE * blk_inode_ix, 0, DEFAULT_INODE_SIZE);

    write_block(vdisk, blk_buf, DEFAULT_BLK_SIZE, blk_ix);
}