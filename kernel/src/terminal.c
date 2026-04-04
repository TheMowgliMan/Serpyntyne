#include <stdarg.h>
#include <limits.h>
#include <stdbool.h>
#include <flanterm.h>
#include <memory.h>
#include <limine.h>
#include <flanterm_backends/fb.h>
#include <terminal.h>

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

struct limine_framebuffer *framebuffer = NULL;
struct flanterm_context *ft_ctx = NULL;

void termInit()
{
    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
        || framebuffer_request.response->framebuffer_count < 1)
    {
        for (;;) {;}
    }

    // Fetch the first framebuffer.
    framebuffer = framebuffer_request.response->framebuffers[0];

    ft_ctx = flanterm_fb_init(
        NULL,
        NULL,
        framebuffer->address, framebuffer->width, framebuffer->height, framebuffer->pitch,
        framebuffer->red_mask_size, framebuffer->red_mask_shift,
        framebuffer->green_mask_size, framebuffer->green_mask_shift,
        framebuffer->blue_mask_size, framebuffer->blue_mask_shift,
        NULL,
        NULL, NULL,
        NULL, NULL,
        NULL, NULL,
        NULL, 0, 0, 1,
        0, 0,
        0,
        0
    );
}

void kputs(const char* msg)
{
    flanterm_write(ft_ctx, msg, strlen(msg));
}

void kerror(const char *msg)
{
    kputs("\033[0;31m");
    kputs(msg);
    kputs("\033[0;37m"); // We assume that text is normally white
}

char kputchar(int c)
{
    char cc = (char)c;
    flanterm_write(ft_ctx, &cc, 1);
    return cc;
}

static bool print(const char* data, size_t length) {
    const unsigned char* bytes = (const unsigned char*) data;
    for (size_t i = 0; i < length; i++)
        if (kputchar(bytes[i]) == EOF)
            return false;
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

            if (decimal < 0)
            {
                decimal *= -1;

                if (maxrem > 0) kputs("-");
            }

            uint64_t divisor = 1000000000000000000; // The largest power of ten in an int64_t
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
                divisor /= 10;
            } while (divisor > 0);
        }
        else if (*format == 'b')
        {
            format++;
            uint64_t binary = (uint64_t)va_arg(parameters, int64_t);

            do
            {
                char c = (char)(binary & 0x1) + 48;

                if (!maxrem)
                {
                    // TODO: Set errno to EOVERFLOW.
                    return -1;
                }

                if (!print(&c, sizeof(c)))
                    return -1;

                written += 1;
                binary = binary >> 1;
            } while (binary > 0);
        }
        else if (*format == 'x')
        {
            format++;
            uint64_t hexadecimal = (uint64_t)va_arg(parameters, int64_t);
            if (maxrem > 2) kputs("0x");

            do
            {
                char c = (char)(hexadecimal & 0xF) + 48;
                if (c > 0x39) c += 39; // For characters a-f

                if (!maxrem)
                {
                    // TODO: Set errno to EOVERFLOW.
                    return -1;
                }

                if (!print(&c, sizeof(c)))
                    return -1;

                written += 1;
                hexadecimal = hexadecimal >> 4;
            } while (hexadecimal > 0);
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
