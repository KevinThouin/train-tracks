#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdlib.h>
#include <limits.h>

#ifdef __cplusplus__
extern "C" {
#endif

#define VERSION "1.0.1"

#ifdef __GNUC__
 #define USE_GCC 1
#else
 #define USE_GCC 0
#endif

#define GCC_VERSION (__GNUC__ * 10000 \
					 + __GNUC_MINOR__ * 100 \
					 + __GNUC_PATCHLEVEL)

#if USE_GCC && GCC_VERSION>40500
 #define UNREACHABLE() __builtin_unreachable()
#else
 #define UNREACHABLE() ((void) 0)
#endif

#if USE_GCC && GCC_VERSION>40600
static inline int msb(size_t v) {
	return sizeof(v)*CHAR_BIT - __builtin_clz(v) - 1;
}
#else
static inline int msb(size_t v) {
	int ret = 0;
	while (v>>=1) {
		++ret;
	}
	return ret;
}
#endif

#ifdef __cplusplus__
}
#endif

#endif /* __UTIL_H__ */
