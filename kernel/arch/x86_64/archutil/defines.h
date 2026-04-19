#ifndef ARCH_DEFINES_H_
#define ARCH_DEFINES_H_

#include <stdint.h>

#define PAGE_SIZE 4096
#define LARGE_PAGE_SIZE 0x200000ull
#define LARGE_PAGE_SIZE_EXPONENT 9

#define LOW_MEM_BOUNDARY 4294967296ull
#define MINIMUM_MEM_BOUNDARY 1048576ull

#define ARCH_WIDTH 8
#define ARCH_POINTER_WIDTH 8
#define ARCH_DATA_WIDTH 8

#define ARCH_NAME "x86-64"

typedef uint64_t phys_addr_t;
typedef uint64_t virt_addr_t;

typedef uint64_t uintarch_t;

#endif
