/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     2013-05-09.
 * Revision 2026-03-29.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Subroutine — function pointer type for native callables and resolvers.
 *
 * Used as:
 *   - The resolver callback passed to PLAIN_WALK (e.g. PLAIN_RESOLVE).
 *   - The native field in PLAIN_CALLABLE (built-in procedure implementations).
 *   - The handler field in PLAIN_CONTEXT (host fallback for unknown keywords).
 */

typedef PLAIN_WORD_DOUBLE (*PLAIN_SUBROUTINE) (void* context, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value);
