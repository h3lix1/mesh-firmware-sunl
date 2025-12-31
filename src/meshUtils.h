#pragma once
#include "DebugConfiguration.h"
#include <algorithm>
#include <cstdarg>
#include <iterator>
#include <stdint.h>

/// C++ v17+ clamp function, limits a given value to a range defined by lo and hi
template <class T> constexpr const T &clamp(const T &v, const T &lo, const T &hi)
{
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

#if HAS_SCREEN
#define IF_SCREEN(X)                                                                                                             \
    if (screen) {                                                                                                                \
        X;                                                                                                                       \
    }
#else
#define IF_SCREEN(...)
#endif

#if (defined(ARCH_PORTDUINO) && !defined(STRNSTR))
#define STRNSTR
#include <string.h>
char *strnstr(const char *s, const char *find, size_t slen);
#endif

#if defined(ARCH_PORTDUINO)
// strlcpy may not be available on all Linux systems
#if __has_include(<bsd/string.h>)
#include <bsd/string.h>
#else
// Fallback implementation if bsd/string.h is not available
#include <string.h>
static inline size_t strlcpy(char *dst, const char *src, size_t size)
{
    size_t src_len = strlen(src);
    if (size > 0) {
        size_t copy_len = (src_len < size - 1) ? src_len : size - 1;
        memcpy(dst, src, copy_len);
        dst[copy_len] = '\0';
    }
    return src_len;
}
#endif
#endif

void printBytes(const char *label, const uint8_t *p, size_t numbytes);

// is the memory region filled with a single character?
bool memfll(const uint8_t *mem, uint8_t find, size_t numbytes);

bool isOneOf(int item, int count, ...);

const std::string vformat(const char *const zcFormat, ...);

#define IS_ONE_OF(item, ...) isOneOf(item, sizeof((int[]){__VA_ARGS__}) / sizeof(int), __VA_ARGS__)
