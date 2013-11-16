/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     02/23/2013,
 * Revision 11/16/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#define MOCOSEL_TARGET_DUMMY        0x00
#define MOCOSEL_TARGET_POSIX        0x01
#define MOCOSEL_TARGET_LINUX        0x02
#define MOCOSEL_TARGET_ANDROID      0x04
#define MOCOSEL_TARGET_DARWIN       0x08
#define MOCOSEL_TARGET_OS_X         0x10
#define MOCOSEL_TARGET_iOS          0x20
#define MOCOSEL_TARGET_SIMULATOR    0x40
#define MOCOSEL_TARGET_WINDOWS      0x80

/* Android. */
#if ANDROID
    #define MOCOSEL_TARGET (MOCOSEL_TARGET_POSIX | MOCOSEL_TARGET_ANDROID)
/* GNU/Linux. */
#elif defined(__linux) || defined(__linux__) || defined(linux)
    #define MOCOSEL_TARGET (MOCOSEL_TARGET_POSIX | MOCOSEL_TARGET_LINUX)
/* Darwin. */
#elif defined(__APPLE__)
    #include "TargetConditionals.h"

    /* iOS Simulator. */
    #if TARGET_OS_IPHONE_SIMULATOR
        #define MOCOSEL_TARGET (MOCOSEL_TARGET_POSIX | MOCOSEL_TARGET_DARWIN | MOCOSEL_TARGET_iOS | MOCOSEL_TARGET_SIMULATOR)
    /* iOS. */
    #elif TARGET_IPHONE
        #define MOCOSEL_TARGET (MOCOSEL_TARGET_POSIX | MOCOSEL_TARGET_DARWIN | MOCOSEL_TARGET_iOS)
    /* OS X. */
    #else
        #define MOCOSEL_TARGET (MOCOSEL_TARGET_POSIX | MOCOSEL_TARGET_DARWIN | MOCOSEL_TARGET_OS_X)
    #endif
/* Microsoft Windows. */
#elif defined(_MSC_VER) || defined(_WIN32)
    #define MOCOSEL_TARGET MOCOSEL_TARGET_WINDOWS
/* Dummy. */
#else
    #define MOCOSEL_TARGET MOCOSEL_TARGET_DUMMY
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
    #if __STDC_VERSION__ >= 199101L
        #define MOCOSEL_INLINE static inline __attribute__((always_inline))
    #else
        #define MOCOSEL_INLINE static
    #endif
#elif MOCOSEL_TARGET & MOCOSEL_TARGET_WINDOWS
    #define MOCOSEL_INLINE static __forceinline
#elif defined(__cplusplus) || (__STDC_VERSION__ >= 199901L)
    #define MOCOSEL_INLINE static inline
#else
    #define MOCOSEL_INLINE static
#endif

/* MOCOSEL_RESTRICT. */
#if MOCOSEL_TARGET & MOCOSEL_TARGET_POSIX
    #define MOCOSEL_RESTRICT __restrict__
#elif MOCOSEL_TARGET & MOCOSEL_TARGET_WINDOWS
    #define MOCOSEL_RESTRICT __restrict
#elif __STDC_VERSION__ >= 199901L
    #define MOCOSEL_RESTRICT restrict
#else
    #define MOCOSEL_RESTRICT
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
