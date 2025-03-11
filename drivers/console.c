#include "hal/console.h"
#include <stdarg.h>

// 初始化控制台
void console_init(void)
{
    platform_console_ops.init();
}

// 输出单个字符
void console_putchar(char c)
{
    platform_console_ops.putchar(c);
}

// 读取单个字符
char console_getchar(void)
{
    return platform_console_ops.getchar();
}

// 批量写入字符
void console_write(const char *s, size_t n)
{
    platform_console_ops.write(s, n);
}

// 批量读取字符
void console_read(char *s, size_t n)
{
    platform_console_ops.read(s, n);
}

// 设置控制台颜色
void console_set_color(console_color_t fg, console_color_t bg)
{
    platform_console_ops.set_color(fg, bg);
}

// 清屏
void console_clear(void)
{
    platform_console_ops.clear();
}

// 格式化打印函数
static void printint(int xx, int base, int sign, int width, char pad)
{
    static char digits[] = "0123456789abcdef";
    char buf[16];
    int i;
    unsigned int x;

    if (sign && (sign = xx < 0))
        x = -xx;
    else
        x = xx;

    i = 0;
    do
    {
        buf[i++] = digits[x % base];
    } while ((x /= base) != 0);

    if (sign)
        buf[i++] = '-';

    while (i < width)
        buf[i++] = pad;

    while (--i >= 0)
        console_putchar(buf[i]);
}

static void printlong(long long xx, int base, int sign)
{
    static char digits[] = "0123456789abcdef";
    char buf[32];
    int i;
    unsigned long long x=0;

    if (sign && (sign = xx < 0))
        x = -xx;
    else
        x = xx;

    i = 0;
    do
    {
        buf[i++] = digits[x % base];
    } while ((x /= base) != 0);

    if (sign)
        buf[i++] = '-';

    while (--i >= 0)
        console_putchar(buf[i]);
}
// 格式化打印实现
void kprintf(const char *fmt, ...)
{
    va_list ap;
    int i, c;
    char *s;

    va_start(ap, fmt);
    for (i = 0; (c = fmt[i] & 0xff) != 0; i++)
    {
        if (c != '%')
        {
            console_putchar(c);
            continue;
        }
        c = fmt[++i] & 0xff;
        if (c == 0)
            break;
        int width = 0;
        char pad = ' ';
        if (c == '0')
        {
            pad = '0';
            c = fmt[++i] & 0xff;
            while (c >= '0' && c <= '9')
            {
                width = width * 10 + (c - '0');
                c = fmt[++i] & 0xff;
            }
        }
        switch (c)
        {
        case 'd':
            printint(va_arg(ap, int), 10, 1, width, pad);
            break;
        case 'x':
        case 'p':
            printint(va_arg(ap, int), 16, 0, width, pad);
            break;
        case 'l':
            if (fmt[i + 1] == 'l' && fmt[i + 2] == 'x')
            {
                i += 2;
                printlong(va_arg(ap, long long), 16, 0);
            }
            break;
        case 's':
            if ((s = va_arg(ap, char *)) == 0)
                s = "(null)";
            for (; *s; s++)
                console_putchar(*s);
            break;
        case '%':
            console_putchar('%');
            break;
        default:
            console_putchar('%');
            console_putchar(c);
            break;
        }
    }
    va_end(ap);
}