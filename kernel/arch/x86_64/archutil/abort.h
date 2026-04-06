#ifndef ABORT_H_
#define ABORT_H_

void kernel_abort(void)
{
    asm volatile ("cli");
    for (;;)
    {
        asm volatile ("hlt");
    }
}

#endif
