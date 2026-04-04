#ifndef TERMINAL_H_
#define TERMINAL_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#define EOF (-1)

void termInit();
void kputs(const char* msg);
void kerror(const char *msg);
char kputchar(int c);
int kprintf(const char* restrict format, ...);

#endif
