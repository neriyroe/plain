/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     2014-10-15.
 * Revision 2026-03-29.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Delegate — callback type for error and diagnostic reporting.
 *
 * Invoked by the tokenizer and runtime when a syntax or evaluation error
 * occurs.  <data> points into the source text at the error location;
 * <length> is the extent of the erroneous fragment; <type> is one of the
 * PLAIN_ERROR_SYNTAX_* codes from Error.h.
 */

typedef void (*PLAIN_DELEGATE) (void* context, const PLAIN_BYTE* data, PLAIN_WORD_DOUBLE length, PLAIN_WORD_DOUBLE type);
