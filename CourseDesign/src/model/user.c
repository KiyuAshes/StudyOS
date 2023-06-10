#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "user.h"
#include "util.h"

// 创建用户
void creat_user(User* ulist, uint16_t* ulist_len, uint16_t uid, char* uname, char* passwd) {
    uint16_t i;

    if (uid == 0) {
        printf("用户创建失败!非法UID!\n");
        return;
    }

    if (*ulist_len >= USER_N_MAX) {
        printf("用户创建失败!用户数量超过限制!\n");
        return;
    }

    for (i = 0; i < *ulist_len; i++) {
        if (ulist[i].uid == uid) {
            printf("用户创建失败!UID重复!\n");
            return;
        }
    }

    ulist[*ulist_len].uid = uid;
    strcpy(ulist[*ulist_len].uname, uname);
    strcpy(ulist[*ulist_len].passwd, passwd);

    (*ulist_len)++;
}

// 切换用户
void switch_user(const User* ulist, uint16_t ulist_len, uint16_t* uid_cur, uint16_t* u_ix_cur) {
    uint16_t ix;
    uint16_t uid;
    char uname[20];
    char passwd[30];

    if (try_read_newline()) {
        ix = 0;
        uid = 0;
    }
    else {
        scanf("%s", uname);

        for (ix = 0; ix < ulist_len; ix++) {
            if (strcmp(uname, ulist[ix].uname) == 0) {
                uid = ulist[ix].uid;
                break;
            }
        }

        if (ix == ulist_len) {
            printf("用户不存在");
            return;
        }
    }

    printf("请输入密码:");
    scanf("%s", passwd);

    if (strcmp(passwd, ulist[ix].passwd) == 0) {
        *uid_cur = uid;
        *u_ix_cur = ix;
    }
}