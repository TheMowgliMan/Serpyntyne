#include <stdint.h>
#include <stddef.h>
#include <memory.h>
#include <archutil/defines.h>

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// They must be implemented as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
    uint8_t *restrict pdest = (uint8_t *restrict)dest;
    const uint8_t *restrict psrc = (const uint8_t *restrict)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    if ((n % ARCH_WIDTH) != 0)
    {
        uint8_t *p = (uint8_t *)s;

        for (size_t i = 0; i < n; i++) {
            p[i] = (uint8_t)c;
        }

        return s;
    } else {
        uintarch_t *p = (uintarch_t*)s;

        uintarch_t tmp = (uintarch_t)c;

        uintarch_t set;
        if (ARCH_WIDTH == 8)
            set = tmp | tmp << 8 | tmp << 16 | tmp << 24 | tmp << 32 | tmp << 40 | tmp << 48 | tmp << 56;

        for (size_t i = 0; i < (n / ARCH_WIDTH); i++)
        {
            p[i] = set;
        }

        return s;
    }
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if ((uintptr_t)src > (uintptr_t)dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if ((uintptr_t)src < (uintptr_t)dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}
