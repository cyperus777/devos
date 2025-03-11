#include "hal/console.h"
#include "x86.h"

#define CGA_BASE    0xb8000
#define CGA_WIDTH   80
#define CGA_HEIGHT  25
#define CGA_ATTR    0x07    // 默认的显示属性：白色文本

static volatile uint16_t* cga_mem = (uint16_t*)CGA_BASE;
static int cursor_x = 0;
static int cursor_y = 0;
static uint8_t current_attr = CGA_ATTR;

// 更新硬件光标位置
static void update_cursor(void) {
    unsigned short position = (cursor_y * CGA_WIDTH) + cursor_x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(position & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((position >> 8) & 0xFF));
}

static void scroll(void) {
    if (cursor_y >= CGA_HEIGHT) {
        // 移动所有行向上一行
        for (int i = 0; i < CGA_HEIGHT - 1; i++) {
            for (int j = 0; j < CGA_WIDTH; j++) {
                cga_mem[i * CGA_WIDTH + j] = cga_mem[(i + 1) * CGA_WIDTH + j];
            }
        }
        // 清除最后一行
        for (int j = 0; j < CGA_WIDTH; j++) {
            cga_mem[(CGA_HEIGHT-1) * CGA_WIDTH + j] = (current_attr << 8) | ' ';
        }
        cursor_y = CGA_HEIGHT - 1;
    }
}

static void x86_console_init(void) {
    cursor_x = 0;
    cursor_y = 0;
    current_attr = CGA_ATTR;
    // 清屏
    for (int i = 0; i < CGA_WIDTH * CGA_HEIGHT; i++) {
        cga_mem[i] = (current_attr << 8) | ' ';
    }
    update_cursor();
}

static void x86_console_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        scroll();
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
        }
    } else {
        cga_mem[cursor_y * CGA_WIDTH + cursor_x] = (current_attr << 8) | c;
        cursor_x++;
        if (cursor_x >= CGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
            scroll();
        }
    }
    update_cursor();
}

static char x86_console_getchar(void) {
    // 从键盘读取字符的实现（需要键盘驱动支持）
    return 0;
}

static void x86_console_write(const char* s, size_t n) {
    for (size_t i = 0; i < n; i++) {
        x86_console_putchar(s[i]);
    }
}

static void x86_console_read(char* s, size_t n) {
    // 读取字符串的实现
}

static void x86_console_set_color(console_color_t fg, console_color_t bg) {
    current_attr = (bg << 4) | (fg & 0x0F);
}

static void x86_console_clear(void) {
    for (int i = 0; i < CGA_WIDTH * CGA_HEIGHT; i++) {
        cga_mem[i] = (current_attr << 8) | ' ';
    }
    cursor_x = cursor_y = 0;
    update_cursor();
}

static void x86_console_get_cursor(int* x, int* y) {
    *x = cursor_x;
    *y = cursor_y;
}

static void x86_console_set_cursor(int x, int y) {
    if (x >= 0 && x < CGA_WIDTH && y >= 0 && y < CGA_HEIGHT) {
        cursor_x = x;
        cursor_y = y;
        update_cursor();
    }
}

// 导出平台实现的接口
const hal_console_ops_t platform_console_ops = {
    .init = x86_console_init,
    .putchar = x86_console_putchar,
    .getchar = x86_console_getchar,
    .write = x86_console_write,
    .read = x86_console_read,
    .set_color = x86_console_set_color,
    .clear = x86_console_clear,
    .get_cursor = x86_console_get_cursor,
    .set_cursor = x86_console_set_cursor
};