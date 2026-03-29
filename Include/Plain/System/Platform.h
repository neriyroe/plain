/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     02/23/2013.
 * Revision 09/02/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 *
 * Platform — target detection, compiler intrinsics, and fundamental types.
 *
 * This is the lowest-level header in the include chain.  Everything else
 * in the library ultimately includes this.  It provides:
 *   - PLAIN_TARGET detection (POSIX, Windows, or standard C fallback).
 *   - Compiler-specific macros: PLAIN_ALIGN, PLAIN_ASSERT, PLAIN_INLINE,
 *     PLAIN_RESTRICT, PLAIN_AUTO (stack allocation).
 *   - Fundamental type aliases used throughout the runtime.
 */

#pragma once

/* ---- Target detection -------------------------------------------- */

#define PLAIN_TARGET_STANDARD     0x00
#define PLAIN_TARGET_POSIX        0x01
#define PLAIN_TARGET_WINDOWS      0x02

#if defined(__linux) || defined(__linux__) || defined(linux)
    #define PLAIN_TARGET PLAIN_TARGET_POSIX
#elif defined(__APPLE__)
    #define PLAIN_TARGET PLAIN_TARGET_POSIX
#elif defined(_MSC_VER) || defined(_WIN32)
    #define PLAIN_TARGET PLAIN_TARGET_WINDOWS
#else
    #define PLAIN_TARGET PLAIN_TARGET_STANDARD
#endif

/* ---- Compiler intrinsics ----------------------------------------- */

/* Structure alignment. */
#if PLAIN_TARGET & PLAIN_TARGET_POSIX
    #define PLAIN_ALIGN(number) __attribute__((aligned(number)))
#elif PLAIN_TARGET & PLAIN_TARGET_WINDOWS
    #define PLAIN_ALIGN(number) __declspec(align(number))
#else
    #define PLAIN_ALIGN(number)
#endif

/* Debug assertion — compiles to nothing unless PLAIN_DEBUGGING is defined. */
#ifdef PLAIN_DEBUGGING
    #define PLAIN_ASSERT(condition) assert(condition)
#else
    #define PLAIN_ASSERT(condition)
#endif

/* Forced inlining. */
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

/* Restrict qualifier. */
#if PLAIN_TARGET & PLAIN_TARGET_POSIX
    #define PLAIN_RESTRICT __restrict__
#elif PLAIN_TARGET & PLAIN_TARGET_WINDOWS
    #define PLAIN_RESTRICT __restrict
#else
    #define PLAIN_RESTRICT restrict
#endif

/* Stack allocation (alloca). */
#if PLAIN_TARGET & PLAIN_TARGET_WINDOWS
    #define PLAIN_AUTO(number) _alloca(number)
#else
    #define PLAIN_AUTO(number) alloca(number)
#endif

/* ---- Standard headers -------------------------------------------- */

#ifdef PLAIN_DEBUGGING
    #include <assert.h>
#endif

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/* ---- Fundamental type aliases ------------------------------------ */

#define PLAIN_BYTE                unsigned char       /* Byte — used for all raw data and strings. */
#define PLAIN_REAL                double              /* Double-precision real — storage type for Plain real values. */
#define PLAIN_REAL_DOUBLE         double              /* Alias kept for source compatibility; same as PLAIN_REAL. */
#define PLAIN_WORD                unsigned short int   /* 16-bit unsigned — not currently used in the runtime. */
#define PLAIN_WORD_DOUBLE         unsigned int         /* 32-bit unsigned — internal lengths, error codes, counters. */
#define PLAIN_WORD_QUADRUPLE      unsigned long long int  /* 64-bit unsigned — storage type for Plain integer values. */
