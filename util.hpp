#ifndef __UTIL_H__
#define __UTIL_H__

#include <cstdlib>
#include <climits>

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

namespace {
	
#if USE_GCC && GCC_VERSION>40600
inline int clz(unsigned int v) {
	return __builtin_clz(v);
}

inline int clz(unsigned long v) {
	return __builtin_clzl(v);
}

inline int clz(unsigned long long v) {
	return __builtin_clzll(v);
}

template <typename INT_TYPE>
int msb(INT_TYPE v) {
	return sizeof(INT_TYPE)*CHAR_BIT - clz(v) - 1;
}

#else
template <typename INT_TYPE>
inline int msb(INT_TYPE v) {
	int ret = 0;
	while (v>>=1) {
		++ret;
	}
	return ret;
}
#endif

}

#endif /* __UTIL_H__ */
