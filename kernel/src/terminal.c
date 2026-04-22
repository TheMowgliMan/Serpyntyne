#include <stdarg.h>
#include <limits.h>
#include <stdbool.h>
#include <flanterm.h>
#include <memory.h>
#include <util/liminereq.h>
#include <flanterm_backends/fb.h>
#include <terminal.h>
#include <util/atomics.h>

spinlock_t terminal_lock;
spinlock_t *term_lock = &terminal_lock;

struct flanterm_context *ft_ctx = NULL;

uint32_t ansi_colors[8] = {
    0x00110d12, // black
    0x00ae2334, // red
    0x001ebc73, // green
    0x00966c6c, // brown
    0x004d65b4, // blue
    0x00831c5d, // magenta
    0x000b8a8f, // cyan
    0x00c7dcd0 // grey
};
uint32_t ansi_bright[8] = {
    0x002e222f,
    0x00e83b3b,
    0x0091db69,
    0x00ab947a,
    0x004d9be6,
    0x00c32454,
    0x000eaf9b,
    0x00ffffff
};

uint32_t default_fg = 0x00c7dcd0;
uint32_t default_bg = 0x00110d12;

uint32_t default_bright_fg = 0x00ffffff;
uint32_t default_bright_bg = 0x003e3546;

void termInit()
{
    ft_ctx = flanterm_fb_init(
        NULL,
        NULL,
        limine_framebuffer_ctx->address, limine_framebuffer_ctx->width, limine_framebuffer_ctx->height, limine_framebuffer_ctx->pitch,
        limine_framebuffer_ctx->red_mask_size, limine_framebuffer_ctx->red_mask_shift,
        limine_framebuffer_ctx->green_mask_size, limine_framebuffer_ctx->green_mask_shift,
        limine_framebuffer_ctx->blue_mask_size, limine_framebuffer_ctx->blue_mask_shift,
        NULL,
        ansi_colors, ansi_bright, // Colors (normal then bright)
        &default_bg, &default_fg, // Default bg, then fb
        &default_bright_bg, &default_bright_fg, // Default bright bg, then bright fb
        NULL, 0, 0, 1,
        0, 0,
        0,
        0
    );

    initSpinlock(term_lock);

    kputs(TTY_RESET);
}

void kputs(const char* msg)
{
    acquireSpinlock(term_lock, 0);
    flanterm_write(ft_ctx, msg, strlen(msg));
    releaseSpinlock(term_lock);
}

void kerror(const char *msg)
{
    kputs(TTY_RED);
    kputs(msg);
    kputs(TTY_RESET); // We assume that text is normally white
}

char kputchar(int c)
{
    char cc = (char)c;
    acquireSpinlock(term_lock, 0);
    flanterm_write(ft_ctx, &cc, 1);
    releaseSpinlock(term_lock);
    return cc;
}

static bool print(const char* data, size_t length) {
    acquireSpinlock(term_lock, 0);
    flanterm_write(ft_ctx, data, length);
    releaseSpinlock(term_lock);
    return true;
}

int kprintf(const char* restrict format, ...) {
    va_list parameters;
    va_start(parameters, format);

    int written = 0;

    while (*format != '\0')
    {
        size_t maxrem = INT_MAX - written;

        if (format[0] != '%' || format[1] == '%')
        {
            if (format[0] == '%')
                format++;
            size_t amount = 1;
            while (format[amount] && format[amount] != '%')
                amount++;
            if (maxrem < amount)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!print(format, amount))
                return -1;
            format += amount;
            written += amount;
            continue;
        }

        const char* format_begun_at = format++;

        if (*format == 'c')
        {
            format++;
            char c = (char) va_arg(parameters, int /* char promotes to int */);
            if (!maxrem) {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!print(&c, sizeof(c)))
                return -1;
            written++;
        }
        else if (*format == 's')
        {
            format++;
            const char* str = va_arg(parameters, const char*);
            size_t len = strlen(str);
            if (maxrem < len) {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!print(str, len))
                return -1;
            written += len;
        }
        else if (*format == 'd')
        {
            format++;
            int64_t decimal = va_arg(parameters, int64_t);

            if (decimal == 0)
            {
                if (maxrem > 0) kputs("0");
                written++;
            }
            else
            {
                if (decimal < 0)
                {
                    decimal *= -1;

                    if (maxrem > 0) kputs("-");
                    written++;
                    maxrem = INT_MAX - written;
                }

                int64_t divisor = 1000000000000000000; // The largest power of ten in an int64_t
                for (; divisor > decimal; divisor /= 10);

                do {
                    char c = (char)((decimal / divisor) % 10) + 48; // This gets us the digit in ASCII

                    if (!maxrem)
                    {
                        // TODO: Set errno to EOVERFLOW.
                        return -1;
                    }

                    if (!print(&c, sizeof(c)))
                        return -1;

                    written += 1;
                    maxrem = INT_MAX - written;
                    divisor /= 10;
                } while (divisor > 0);
            }
        }
        else if (*format == 'b')
        {
            format++;
            uint64_t binary = (uint64_t)va_arg(parameters, int64_t);

            uint8_t digit = 63;
            bool writing = false;

            do
            {
                char c = (char)((binary & (0b1ul << digit)) >> digit);
                c += 48;

                if (!maxrem)
                {
                    // TODO: Set errno to EOVERFLOW.
                    return -1;
                }

                if (c != '0' || writing == true || digit == 0) {
                    if (!print(&c, sizeof(c)))
                        return -1;

                    written += 1;
                    writing = true;
                }

                digit -= 1;
            } while (digit != UINT8_MAX);
        }
        else if (*format == 'x')
        {
            format++;
            uint64_t hexadecimal = (uint64_t)va_arg(parameters, int64_t);

            uint8_t digit = 15;
            bool writing = false;

            if (maxrem > 1) kputs("0x");

            do
            {
                char c = (char)((hexadecimal & (0xFul << (digit * 4))) >> (digit * 4));
                c += 48;

                if (c > 0x39) c += 39; // For characters a-f

                if (!maxrem)
                {
                    // TODO: Set errno to EOVERFLOW.
                    return -1;
                }

                if (c != '0' || writing == true || digit == 0) {
                    if (!print(&c, sizeof(c)))
                        return -1;

                    writing = true;
                }

                written += 1;
                digit -= 1;
            } while (digit != UINT8_MAX);
        }
        else
        {
            format = format_begun_at;
            size_t len = strlen(format);
            if (maxrem < len) {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!print(format, len))
                return -1;
            written += len;
            format += len;
        }
    }

    va_end(parameters);
    return written;
}
