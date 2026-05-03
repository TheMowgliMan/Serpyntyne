#ifndef ARCH_DEFINES_H_
#define ARCH_DEFINES_H_

#include <stdint.h>

#define PAGE_SIZE 4096
#define PAGE_SIZE_EXPONENT 0
#define LARGE_PAGE_SIZE 2097152ull
#define LARGE_PAGE_SIZE_EXPONENT 9

#define LOW_MEM_BOUNDARY 4294967296ull
#define MINIMUM_MEM_BOUNDARY 1048576ull
#define TAR 2097152

#define ARCH_WIDTH 8
#define ARCH_POINTER_WIDTH 8
#define ARCH_DATA_WIDTH 8

#define ARCH_HEAP_ALLOCATE_SIZE LARGE_PAGE_SIZE
#define ARCH_HEAP_ALLOCATE_SIZE_EXPONENT LARGE_PAGE_SIZE_EXPONENT
#define ARCH_HEAP_MAP_IF_LARGER_SIZE PAGE_SIZE

#define ARCH_NAME "x86-64"

typedef uint64_t phys_addr_t;
typedef uint64_t virt_addr_t;

typedef uint64_t uintarch_t;

#define PAGEEXP_TO_DIRECT(X) ((uint64_t)((1ull << X) * 4096))

#endif
