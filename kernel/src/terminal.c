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
