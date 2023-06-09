#include <bits/stdint-uintn.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

// 输入输出
uint8_t try_read_newline() {
    char c;

    c = getchar();

    if (c != '\n') {
        ungetc(c, stdin);
        return 0;
    }

    return 1;
}

// 字符串处理
uint8_t no_end_strcmp(const char* s1, const char* s2, uint32_t l1, uint32_t l2) {
    if (l1 != l2) {
        return 0;
    }

    for (int i = 0; i < l1; i++) {
        if (s1[i] != s2[i]) {
            return 0;
        }
    }

    return 1;
}

// BitMap数组操作
void set_bit(char* bit_arr, uint32_t offset) {
    bit_arr[offset / 8] = bit_arr[offset / 8] | (1 << (offset % 8));
}

void reset_bit(char* bit_arr, uint32_t offset) {
    bit_arr[offset / 8] = bit_arr[offset / 8] & ~(1 << (offset % 8));
}

uint8_t get_bit(const char* bit_arr, uint32_t offset) {
    return (bit_arr[offset / 8] >> (offset % 8)) & 1;
}