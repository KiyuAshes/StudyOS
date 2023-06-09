#include <stdio.h>
#include <stdint.h>

#ifndef EXT2_H
#    define EXT2_H

//注释(不支持)表示本程序没有利用和支持该值控制的功能

#    define DEFAULT_BLK_SIZE 4096   //块大小,单位字节;本程序实际只支持4K大小的块
#    define DEFAULT_INODE_SIZE 256  // Inode大小,单位字节;本程序实际只支持256B大小的Inode

//超级块结构体
typedef struct SuperBlock
{
    // 通用部分
    uint32_t inodes_count;       // Inodes节点总数
    uint32_t blocks_count;       // Block块总数
    uint32_t r_blocks_count;     // 系统保留Block块数
    uint32_t free_blocks_count;  // 空闲Block块数
    uint32_t free_inodes_count;  // 空闲Inodes节点数
    uint32_t first_data_block;   // 文件系统的第一个块(超级)块号
    uint32_t log_block_size;     // 块大小(不支持)
    uint32_t log_frag_size;      // 片段大小(不支持)
    uint32_t blocks_per_group;   // 每块组块数
    uint32_t frags_per_group;    // 每块组片段数(不支持)
    uint32_t inodes_per_group;   // 每块组Inodes节点数
    uint32_t mtime;              // 最后挂载时间(不支持)
    uint32_t wtime;              // 最后写入时间(不支持)
    uint16_t mnt_count;          // 自上次完全校验后挂载次数(不支持)
    uint16_t max_mnt_count;      // 超过该值需进行完全校验(不支持)
    uint16_t magic;              // 文件系统魔数
    uint16_t state;              // 卸载确认标志(不支持)
    uint16_t errors;             // 非正常卸载时行为(不支持)
    uint16_t minor_rev_level;    // 文件系统小版本号
    uint32_t lastcheck;          // 最后一次检查的时间(不支持)
    uint32_t checkinterval;      // 最大文件系统检查时间间隔(不支持)
    uint32_t creator_os;         // 创建文件系统的操作系统(不支持)
    uint32_t rev_level;          // 修订版本
    uint16_t def_resuid;         // 保留块默认UID,默认0(不支持)
    uint16_t def_resgid;         // 保留块默认GID,默认0(不支持)

    // EXT2_DYNAMIC_REV
    // 可变inode大小和扩展属性部分
    uint32_t first_ino;               // 第一个非保留Inodes节点号
    uint16_t inode_size;              // Inodes节点体大小(不支持)
    uint16_t block_group_nr;          // 该超级块所在块组号(不支持)
    uint32_t feature_compat;          // 兼容特性集，系统可自由选择支持(不支持)
    uint32_t feature_incompat;        // 强制兼容特性集，系统不支持就不应该挂载(不支持)
    uint32_t feature_ro_compat;       // 只读兼容特性集，系统不支持就只能只读挂载(不支持)
    uint8_t uuid[16];                 // 16字节的分区UUID(不支持)
    char volume_name[16];             // 分区名，最长16个字符(不支持)
    char last_mounted[64];            // 上一次的挂载点，最长64个字符(不支持)
    uint32_t algorithm_usage_bitmap;  // 压缩算法(不支持)

    // 性能提示
    // 只在EXT2_COMPAT_PREALLOC 打开时生效
    // 该部分本程序都不支持
    uint8_t prealloc_blocks;      // 创建常规文件时预分配的块数(不支持)
    uint8_t prealloc_dir_blocks;  // 创建目录文件时预分配的块数(不支持)
    uint16_t pad1;                // 对齐填充(不支持)

    // 日志支持
    // 只在 EXT3_FEATURE_COMPAT_HAS_JOURNAL 置位时生效
    // 该部分本程序都不支持
    uint8_t journal_uuid[16];  // 日志超级块UUID(不支持)
    uint32_t journal_inum;     // 日志文件索引号(不支持)
    uint32_t journal_dev;      // 日志文件设备号(不支持)
    uint32_t last_orphan;      // 指向待删除Inodes节点表头的指针(不支持)

    // 目录索引支持
    // 该部分本程序都不支持
    uint32_t hash_seed[4];       // 目录索引算法的hash种子(不支持)
    uint8_t def_hash_version;    // 目录索引hash版本号(不支持)
    uint8_t reserved_char_pad;   // 对齐填充,保留字节
    uint16_t reserved_word_pad;  // 对齐填充,保留字

    // 其他选项
    // 该部分本程序都不支持
    uint32_t default_mount_opts;  // 默认安装选项(不支持)
    uint32_t first_meta_bg;       // 第一个元数据块组ID仅限 Ext3的扩展(不支持)
    uint8_t reserved[760];        // 块尾填充,补齐到1K,保留字段
} SuperBlock;

// 块组描述符结构体
typedef struct GroupDesc
{
    uint32_t bg_block_bitmap;       // 该块组块位图的块号
    uint32_t bg_inode_bitmap;       // 该块组Inodes节点位图的块号
    uint32_t bg_inode_table;        // 该块组Inodes节点表的块号
    uint16_t bg_free_blocks_count;  // 该块组空闲块数
    uint16_t bg_free_inodes_count;  // 该块组Inodes节点数
    uint16_t bg_used_dirs_count;    // 分配给组目录的Inodes节点数
    uint16_t bg_pad;                // 4字节对齐填充
    uint32_t bg_reserved[3];        // 尾部填充,补齐到32字节,保留字段
} GroupDesc;

// Inodes节点结构体
typedef struct Inode
{
    uint16_t i_mode;         // 文件格式和访问权限
    uint16_t i_uid;          // 所有者UID
    uint32_t i_size;         // 文件大小，单位:字节
    uint32_t i_atime;        // 最后访问时间(不支持)
    uint32_t i_ctime;        // 创建时间(不支持)
    uint32_t i_mtime;        // 最后修改时间(不支持)
    uint32_t i_dtime;        // 最后删除时间(不支持)
    uint16_t i_gid;          // 所有者GID(不支持)
    uint16_t i_links_count;  // 硬连接数
    uint32_t i_blocks;       // 占用的块的总数,块大小:512字节
    uint32_t i_flags;        // 文件操作标志(不支持)
    uint32_t osd1;           // 系统相关值1(不支持)
    uint32_t i_block[15];    // 块指针(只支持直接指针)
    uint32_t i_generation;   // 文件版本，用于NFS(不支持)
    uint32_t i_file_acl;     // 文件ACL块指针(不支持)
    uint32_t i_dir_acl;      // 目录ACL块指针(不支持)
    uint32_t i_faddr;        // 段地址(不支持)
    uint32_t osd2[3];        // 系统相关值2(不支持)
} Inode;

//目录条目结构体
typedef struct DirEntry
{
    uint32_t inode;     //目录条目Inodes节点
    uint16_t rec_len;   //目录条目长度
    uint8_t name_len;   //目录条目名长度
    uint8_t file_type;  //文件类型
    char name[255];     //目录条目名
} DirEntry;

enum {
    FT_UNKNOWN,  //未知文件类型
    FT_FILE,     //普通文件类型
    FT_DIR,      //目录文件类型
};

void get_inode(FILE* vdisk, const SuperBlock* super_blk, const GroupDesc* gdt, uint32_t inode_ix, Inode* inode_buf);
void put_inode(FILE* vdisk, const SuperBlock* super_blk, const GroupDesc* gdt, uint32_t inode_ix, const Inode* inode_buf);

uint32_t get_next_free_inode_ix(FILE* vdisk, const SuperBlock* super_blk, const GroupDesc* gdt);
uint32_t get_next_free_block_ix(FILE* vdisk, const SuperBlock* super_blk, const GroupDesc* gdt);

uint32_t get_free_inode(FILE* vdisk, const SuperBlock* super_blk, const GroupDesc* gdt);

void release_inode(FILE* vdisk, const SuperBlock* super_blk, const GroupDesc* gdt, uint32_t inode_ix);
#endif
