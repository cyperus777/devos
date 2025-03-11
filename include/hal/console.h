#ifndef HAL_CONSOLE_H
#define HAL_CONSOLE_H

#include <stddef.h>

// 控制台颜色定义
typedef enum {
    CONSOLE_BLACK = 0,
    CONSOLE_BLUE = 1,
    CONSOLE_GREEN = 2,
    CONSOLE_CYAN = 3,
    CONSOLE_RED = 4,
    CONSOLE_MAGENTA = 5,
    CONSOLE_BROWN = 6,
    CONSOLE_LIGHT_GRAY = 7,
} console_color_t;

// 控制台硬件抽象接口
typedef struct {
    void (*init)(void);                    // 初始化控制台
    void (*putchar)(char c);               // 输出单个字符
    char (*getchar)(void);                 // 读取单个字符
    void (*write)(const char* s, size_t n); // 批量写入
    void (*read)(char* s, size_t n);       // 批量读取
    void (*set_color)(console_color_t fg, console_color_t bg); // 设置颜色
    void (*clear)(void);                   // 清屏
    void (*get_cursor)(int* x, int* y);    // 获取光标位置
    void (*set_cursor)(int x, int y);      // 设置光标位置
} hal_console_ops_t;

// 平台需要实现的接口结构体
extern const hal_console_ops_t platform_console_ops;

// 通用控制台API
void console_init(void);
void console_putchar(char c);
char console_getchar(void);
void console_write(const char* s, size_t n);
void console_read(char* s, size_t n);
void console_set_color(console_color_t fg, console_color_t bg);
void console_clear(void);
void kprintf(const char* fmt, ...);

#endif