/*
 * @Author: KiyuAshes
 * @Date: 2023-05-16 20:47:26
 * @LastEditTime: 2023-06-09 02:37:03
 * @Description:
 * @E-mail: kiyuashes@stu.usc.edu.cn
 * Copyright (c) 2023 by KiyuAshes, All Rights Reserved.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "file.h"
#include "disk.h"
#include "ext2.h"
#include "user.h"
#include "util.h"

int main() {
    // 1.加载镜像文件

    FILE* vd;                                // 虚拟硬盘文件
    char vd_path[512] = "../data/disk.img";  // 虚拟磁盘路径

    printf("请输入虚拟磁盘位置(默认为 ../data/disk.img):");

    // 判断是否为默认路径
    if (!try_read_newline()) {
        scanf("%s", vd_path);
    }

    vd = fopen(vd_path, "rb+");
    if (vd) {
        printf("文件打开成功!\n");
    }
    else {
        printf("文件打开失败!\n");
        return 1;
    }

    // 2.装载超级块

    char blk_buf[DEFAULT_BLK_SIZE];  // 通用块缓冲区

    read_block(vd, blk_buf, DEFAULT_BLK_SIZE, 0);
    SuperBlock super_blk = *(SuperBlock*)(blk_buf + 0x400);

    // 魔数校验
    if (super_blk.magic != 0xEF53) {
        printf("文件检查出错!不是EXT2镜像或块大小不为4K!\n");
        return 1;
    }

    // 2.装载组描述表
    GroupDesc* gdt;    // 块组表
    uint32_t gdt_len;  // 块组表长度

    gdt_len = super_blk.blocks_count / super_blk.blocks_per_group;
    gdt = malloc(gdt_len * sizeof(GroupDesc));

    read_block(vd, blk_buf, DEFAULT_BLK_SIZE, 1);
    memcpy(gdt, blk_buf, gdt_len * sizeof(GroupDesc));

    // 3.装载根目录Inode
    Inode inode_buf;       // Inode节点缓冲区
    Inode inode_f_cur;     // 当前文件节点
    Inode inode_dir_cur;   // 当前目录节点
    Inode inode_dir_root;  // 根目录节点

    get_inode(vd, &super_blk, gdt, 2, &inode_dir_root);
    inode_dir_cur = inode_dir_root;

    // 4.创建内置用户
    uint16_t uid_cur;   // 当前用户uid
    uint16_t u_ix_cur;  // 当前用户索引
    uint16_t ulist_len;
    User ulist[USER_N_MAX];

    ulist_len = 0;

    // 创建root用户
    ulist[0].uid = 0;
    strcpy(ulist[0].uname, "root");
    strcpy(ulist[0].passwd, "123456");
    ulist_len++;

    // 创建管理员用户
    creat_user(ulist, &ulist_len, 1, "admin", "admin");

    uid_cur = 1;
    u_ix_cur = 1;

    // 5.交互循环
    char* f_buf;           // 文件缓冲区
    uint64_t offset;       // 文件偏移
    char f_name_buf[256];  // 文件名缓冲区

    char commond[10];    // 命令缓冲区
    char path_cur[512];  // 当前路径
    char path_buf[512];  // 路径缓冲区

    DirEntry de_cur;  //目录项
    DirEntry de_buf;  //目录项缓冲区
    /*
     * 命令列表
     * 退出软件: exit
     * 切换用户: su [用户名]
     * 查询目录: ls
     * 切换目录: cd    [路径]
     * 建立文件: touch 文件名
     * 删除文件: rm    文件名
     * 打开文件: open  文件名
     * 关闭文件: close 文件名
     * 读取文件: read  文件名               注: 只支持英文文本文件，输出到控制台
     * 写入文件: write 文件名 偏移 字符串    注: 只支持英文文本文件，输出到控制台
     */

    strcpy(path_cur, "/");

    while (1) {
        // 命令提示符
        printf("%s@host:%s$ ", ulist[u_ix_cur].uname, path_cur);

        // 输入命令
        if (try_read_newline()) {
            continue;
        }
        else {
            scanf("%s", commond);
        }

        // 命令判断
        if (strcmp(commond, "exit") == 0) {
            break;
        }
        else if (strcmp(commond, "su") == 0) {
            switch_user(ulist, ulist_len, &uid_cur, &u_ix_cur);
        }
        else if (strcmp(commond, "ls") == 0) {
            uint16_t i;

            f_buf = malloc(inode_dir_cur.i_size * sizeof(char));

            // 读取目录文件
            read_file(vd, &inode_dir_cur, f_buf, inode_dir_cur.i_size);

            offset = 0;

            while (offset < inode_dir_cur.i_size) {
                de_cur = *(DirEntry*)(f_buf + offset);

                // 打印文件类型
                switch (de_cur.file_type) {
                    case FT_FILE: printf("f"); break;
                    case FT_DIR: printf("d"); break;
                    default: printf("-"); break;
                }

                // 打印读写权限
                get_inode(vd, &super_blk, gdt, de_cur.inode, &inode_buf);

                if ((inode_buf.i_mode >> 8) & 1) {
                    printf("r");
                }
                else {
                    printf("-");
                }

                if ((inode_buf.i_mode >> 7) & 1) {
                    printf("w");
                }
                else {
                    printf("-");
                }

                if ((inode_buf.i_mode >> 6) & 1) {
                    printf("x");
                }
                else {
                    printf("-");
                }

                printf(" ");

                // 打印用户名
                for (i = 0; i < ulist_len; i++) {
                    if (ulist[i].uid == inode_buf.i_uid) {
                        printf("%s", ulist[i].uname);
                        break;
                    }
                }

                if (i == ulist_len) {
                    printf("unknow");
                }

                printf("\t");

                // 打印文件名
                for (i = 0; i < de_cur.name_len; i++) {
                    printf("%c", de_cur.name[i]);
                }

                printf("\n");

                // 指向下一个目录条目
                offset += de_cur.rec_len;
            }

            free(f_buf);
        }
        else if (strcmp(commond, "cd") == 0) {
            uint32_t ps, pe;   // 路径处理指针
            uint8_t flag_fin;  // 命令完成指针

            if (!try_read_newline()) {
                scanf("%s", path_buf);

                if (path_buf[0] == '/') {
                    ps = 1;
                    pe = 1;
                    inode_buf = inode_dir_root;
                }
                else {
                    ps = 0;
                    pe = 0;
                    inode_buf = inode_dir_cur;
                }

                while (strlen(path_buf) - 1 != 0 && path_buf[strlen(path_buf) - 1] == '/') {
                    path_buf[strlen(path_buf) - 1] = '\0';
                }

                flag_fin = 0;

                while (path_buf[pe] != '\0') {
                    if (path_buf[pe] == '/') {
                        f_buf = malloc(inode_dir_cur.i_size * sizeof(char));

                        // 读取目录文件
                        read_file(vd, &inode_buf, f_buf, inode_buf.i_size);

                        offset = 0;

                        while (offset < inode_dir_cur.i_size) {
                            de_cur = *(DirEntry*)(f_buf + offset);
                            if (no_end_strcmp(de_cur.name, path_buf + ps, de_cur.name_len, pe - ps)) {
                                break;
                            }
                            offset += de_cur.rec_len;
                        }

                        if (offset >= inode_dir_cur.i_size) {
                            printf("路径不存在!\n");
                            flag_fin = 1;
                            break;
                        }

                        if (de_cur.file_type != FT_DIR) {
                            printf("该路径不是文件夹!\n");
                            flag_fin = 1;
                            break;
                        }

                        get_inode(vd, &super_blk, gdt, de_cur.inode, &inode_buf);
                        ps = pe + 1;
                        pe = ps;

                        free(f_buf);
                    }
                    else {
                        pe++;
                    }
                }

                // 处理最后一段路径
                if (!flag_fin) {
                    f_buf = malloc(inode_dir_cur.i_size * sizeof(char));

                    // 读取目录文件
                    read_file(vd, &inode_buf, f_buf, inode_buf.i_size);

                    offset = 0;

                    while (offset < inode_dir_cur.i_size) {
                        de_cur = *(DirEntry*)(f_buf + offset);
                        if (no_end_strcmp(de_cur.name, path_buf + ps, de_cur.name_len, pe - ps)) {
                            break;
                        }
                        offset += de_cur.rec_len;
                    }

                    if (offset >= inode_dir_cur.i_size) {
                        printf("路径不存在!\n");
                        flag_fin = 1;
                        break;
                    }

                    if (de_cur.file_type != FT_DIR) {
                        printf("该路径不是文件夹!\n");
                        flag_fin = 1;
                        break;
                    }

                    // 更新当前路径和Inode记录

                    if (strcmp(path_cur, "/") == 0) {
                        path_cur[0] = '\0';
                    }

                    if (path_buf[0] == '/') {
                        strcpy(path_cur, path_buf);
                    }
                    else {
                        path_cur[strlen(path_cur)] = '/';
                        path_cur[strlen(path_cur) + 1] = '\0';
                        strcat(path_cur, path_buf);
                    }

                    get_inode(vd, &super_blk, gdt, de_cur.inode, &inode_dir_cur);

                    free(f_buf);
                }
            }
        }
        else if (strcmp(commond, "touch") == 0) {
            if (!try_read_newline()) {
                scanf("%s", f_name_buf);

                memset(&de_buf, 0, sizeof(de_buf));
                memset(&inode_buf, 0, sizeof(inode_buf));

                inode_buf.i_mode = 0b111000000;  // 权限为读写执行
                inode_buf.i_uid = uid_cur;
                inode_buf.i_size = 0;

                de_buf.inode = get_free_inode(vd, &super_blk, gdt);
                de_buf.rec_len = 8 + strlen(f_name_buf) + 1 + (4 - ((strlen(f_name_buf) + 1) % 4));
                de_buf.name_len = strlen(f_name_buf) + 1;
                de_buf.file_type = FT_FILE;
                strcpy(de_buf.name, f_name_buf);

                put_inode(vd, &super_blk, gdt, de_buf.inode, &inode_buf);

                // 目录操作
                f_buf = malloc(inode_dir_cur.i_size * sizeof(char));

                read_file(vd, &inode_dir_cur, f_buf, inode_dir_cur.i_size);  // 读取目录文件

                while (offset < inode_dir_cur.i_size) {
                    de_cur = *(DirEntry*)(f_buf + offset);

                    if ((de_cur.rec_len - 8 - de_cur.name_len - 1) > de_buf.rec_len) {
                        de_buf.rec_len = de_cur.rec_len - 8 - de_cur.name_len - 1 - (4 - ((de_cur.name_len) % 4));

                        de_cur.rec_len = 8 + de_cur.name_len + 1 + (4 - ((de_cur.name_len) % 4));

                        memcpy(f_buf + offset, &de_cur, de_cur.rec_len);
                        memcpy(f_buf + offset + de_cur.rec_len, &de_buf, de_buf.rec_len);

                        write_file(vd, &inode_dir_cur, f_buf, inode_dir_cur.i_size);

                        break;
                    }

                    // 指向下一个目录条目
                    offset += de_cur.rec_len;
                }

                free(f_buf);
            }
        }
        else if (strcmp(commond, "rm") == 0) {
            if (!try_read_newline()) {
                scanf("%s", f_name_buf);

                // 目录操作
                f_buf = malloc(inode_dir_cur.i_size * sizeof(char));

                read_file(vd, &inode_dir_cur, f_buf, inode_dir_cur.i_size);  // 读取目录文件

                while (offset < inode_dir_cur.i_size) {
                    de_cur = *(DirEntry*)(f_buf + offset);

                    if ((de_cur.rec_len - 8 - de_cur.name_len - 1) > de_buf.rec_len) {
                        de_buf.rec_len = de_cur.rec_len - 8 - de_cur.name_len - 1 - (4 - ((de_cur.name_len) % 4));

                        de_cur.rec_len = 8 + de_cur.name_len + 1 + (4 - ((de_cur.name_len) % 4));

                        memcpy(f_buf + offset, &de_cur, de_cur.rec_len);
                        memcpy(f_buf + offset + de_cur.rec_len, &de_buf, de_buf.rec_len);

                        write_file(vd, &inode_dir_cur, f_buf, inode_dir_cur.i_size);

                        break;
                    }

                    // 指向下一个目录条目
                    offset += de_cur.rec_len;
                }

                free(f_buf);
            }
        }
        else if (strcmp(commond, "open") == 0) {
        }
        else if (strcmp(commond, "close") == 0) {
        }
        else if (strcmp(commond, "read") == 0) {
        }
        else if (strcmp(commond, "write") == 0) {
        }
        else {
            printf("未知命令,请重新输入!\n");
        }

        // 清除换行
        getchar();
    }

    // 6.释放内存，关闭文件
    free(gdt);
    fclose(vd);

    return 0;
}