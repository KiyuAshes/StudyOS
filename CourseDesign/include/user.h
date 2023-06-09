#include <stdint.h>

#define USER_N_MAX 32

typedef struct User
{
    uint16_t uid;
    char uname[20];
    char passwd[30];
} User;

void creat_user(User* ulist, uint16_t* ulist_len, uint16_t uid, char* uname, char* passwd);
void switch_user(const User* ulist, uint16_t ulist_len, uint16_t* uid_cur, uint16_t* u_ix_curd);