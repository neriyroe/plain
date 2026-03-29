/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     10/15/2014.
 * Revision 02/09/2016.
 *
 * Copyright 2016 Nerijus Ramanauskas.
 *
 * Delegate — callback type for error and diagnostic reporting.
 *
 * Invoked by the tokenizer and runtime when a syntax or evaluation error
 * occurs.  <data> points into the source text at the error location;
 * <length> is the extent of the erroneous fragment; <type> is one of the
 * PLAIN_ERROR_SYNTAX_* codes from Error.h.
 */

typedef void (*PLAIN_DELEGATE) (void* context, const PLAIN_BYTE* data, PLAIN_WORD_DOUBLE length, PLAIN_WORD_DOUBLE type);
