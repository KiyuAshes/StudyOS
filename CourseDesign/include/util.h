#include <stdint.h>

// 输入输出
uint8_t try_read_newline();

// 字符串处理
uint8_t no_end_strcmp(const char* s1, const char* s2, uint32_t l1, uint32_t l2);


// BitMap数组操作
void set_bit(char* bit_arr, uint32_t offset);
void reset_bit(char* bit_arr, uint32_t offset);
uint8_t get_bit(const char* bit_arr, uint32_t offset);