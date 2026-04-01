#ifndef TERMINAL_H_
#define TERMINAL_H_

#include <stdint.h>
#include <stddef.h>
#include <limine.h>

void termInit();
void kputs(const char* msg);
void kerror(const char *msg);

#endif
