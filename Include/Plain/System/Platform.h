/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     02/23/2013.
 * Revision 09/02/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

#define PLAIN_TARGET_STANDARD     0x00
#define PLAIN_TARGET_POSIX        0x01
#define PLAIN_TARGET_WINDOWS      0x02

/* GNU/Linux. */
#if defined(__linux) || defined(__linux__) || defined(linux)
    #define PLAIN_TARGET PLAIN_TARGET_POSIX
/* Darwin. */
#elif defined(__APPLE__)
    #define PLAIN_TARGET PLAIN_TARGET_POSIX
/* Microsoft Windows. */
#elif defined(_MSC_VER) || defined(_WIN32)
    #define PLAIN_TARGET PLAIN_TARGET_WINDOWS
/* Standard. */
#else
    #define PLAIN_TARGET PLAIN_TARGET_STANDARD
#endif

/* PLAIN_ALIGN. */
#if PLAIN_TARGET & PLAIN_TARGET_POSIX
    #define PLAIN_ALIGN(number) __attribute__((aligned(number)))
#elif PLAIN_TARGET & PLAIN_TARGET_WINDOWS
    #define PLAIN_ALIGN(number) __declspec(align(number))
#else
    #define PLAIN_ALIGN(number)
#endif

/* PLAIN_ASSERT. */
#ifdef PLAIN_DEBUGGING
    #define PLAIN_ASSERT(condition) assert(condition)
#else
    #define PLAIN_ASSERT(condition)
#endif

/* PLAIN_INLINE. */
#if PLAIN_TARGET & PLAIN_TARGET_POSIX
    #if defined(__cplusplus)
        #define PLAIN_INLINE static inline __attribute__((always_inline))
    #else
        #define PLAIN_INLINE static __inline__ __attribute__((always_inline))
    #endif
#elif PLAIN_TARGET & PLAIN_TARGET_WINDOWS
    #define PLAIN_INLINE static __forceinline
#else
    #define PLAIN_INLINE static inline
#endif

/* PLAIN_RESTRICT. */
#if PLAIN_TARGET & PLAIN_TARGET_POSIX
    #define PLAIN_RESTRICT __restrict__
#elif PLAIN_TARGET & PLAIN_TARGET_WINDOWS
    #define PLAIN_RESTRICT __restrict
#else
    #define PLAIN_RESTRICT restrict
#endif

/* PLAIN_AUTO. */
#if PLAIN_TARGET & PLAIN_TARGET_WINDOWS
    #define PLAIN_AUTO(number) _alloca(number)
#else
    #define PLAIN_AUTO(number) alloca(number)
#endif

 /* Standard. */
#ifdef PLAIN_DEBUGGING
    #include <assert.h>
#endif

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define PLAIN_BYTE                unsigned char
#define PLAIN_REAL                float
#define PLAIN_REAL_DOUBLE         double
#define PLAIN_WORD                unsigned short int
#define PLAIN_WORD_DOUBLE         unsigned int
#define PLAIN_WORD_QUADRUPLE      unsigned long long int
