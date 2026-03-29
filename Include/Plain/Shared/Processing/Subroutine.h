/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     05/09/2013.
 * Revision 03/28/2026.
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
