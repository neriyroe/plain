/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     02/23/2013,
 * Revision 07/20/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#define MOCOSEL_TARGET_STANDARD     0x00
#define MOCOSEL_TARGET_POSIX        0x01
#define MOCOSEL_TARGET_WINDOWS      0x02

/* GNU/Linux. */
#if defined(__linux) || defined(__linux__) || defined(linux)
    #define MOCOSEL_TARGET MOCOSEL_TARGET_POSIX
/* Darwin. */
#elif defined(__APPLE__)
    #define MOCOSEL_TARGET MOCOSEL_TARGET_POSIX
/* Microsoft Windows. */
#elif defined(_MSC_VER) || defined(_WIN32)
    #define MOCOSEL_TARGET MOCOSEL_TARGET_WINDOWS
/* Standard. */
#else
    #define MOCOSEL_TARGET MOCOSEL_TARGET_STANDARD
#endif

/* MOCOSEL_ALIGN. */
#if MOCOSEL_TARGET & MOCOSEL_TARGET_POSIX
    #define MOCOSEL_ALIGN(number) __attribute__((aligned(number)))
#elif MOCOSEL_TARGET & MOCOSEL_TARGET_WINDOWS
    #define MOCOSEL_ALIGN(number) __declspec(align(number))
#else
    #define MOCOSEL_ALIGN(number)
#endif

/* MOCOSEL_ASSERT. */
#ifdef MOCOSEL_DEBUGGING
    #define MOCOSEL_ASSERT(condition) assert(condition)
#else
    #define MOCOSEL_ASSERT(condition)
#endif

/* MOCOSEL_INLINE. */
#if MOCOSEL_TARGET & MOCOSEL_TARGET_POSIX
    #if defined(__cplusplus)
        #define MOCOSEL_INLINE static inline __attribute__((always_inline))
    #else
        #define MOCOSEL_INLINE static __inline__ __attribute__((always_inline))
    #endif
#elif MOCOSEL_TARGET & MOCOSEL_TARGET_WINDOWS
    #define MOCOSEL_INLINE static __forceinline
#else
    #define MOCOSEL_INLINE static inline
#endif

/* MOCOSEL_RESTRICT. */
#if MOCOSEL_TARGET & MOCOSEL_TARGET_POSIX
    #define MOCOSEL_RESTRICT __restrict__
#elif MOCOSEL_TARGET & MOCOSEL_TARGET_WINDOWS
    #define MOCOSEL_RESTRICT __restrict
#else
    #define MOCOSEL_RESTRICT restrict
#endif

/* MOCOSEL_AUTO. */
#if MOCOSEL_TARGET & MOCOSEL_TARGET_WINDOWS
    #define MOCOSEL_AUTO(NUMBER) _alloca(NUMBER)
#else
    #define MOCOSEL_AUTO(NUMBER) alloca(NUMBER)
#endif

 /* Standard. */
#ifdef MOCOSEL_DEBUGGING
    #include <assert.h>
#endif

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define MOCOSEL_BYTE                unsigned char
#define MOCOSEL_REAL                float
#define MOCOSEL_REAL_DOUBLE         double
#define MOCOSEL_WORD                unsigned short int
#define MOCOSEL_WORD_DOUBLE         unsigned int
#define MOCOSEL_WORD_QUADRUPLE      unsigned long long int
