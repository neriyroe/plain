/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     05/25/2014.
 * Revision 09/01/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

#pragma once

/* C++. */
#ifdef __cplusplus
extern "C" {
#endif

#include "System/Platform.h"
#include "Shared/Segment.h"

/* Appends <source> to <destination>. Note that <destination> will be reallocated. */
PLAIN_WORD_DOUBLE PLAIN_CONCAT(struct PLAIN_SEGMENT* destination, PLAIN_WORD_DOUBLE length, const PLAIN_BYTE* source);

/* Returns 32-bit hash of <data>. */
PLAIN_WORD_DOUBLE PLAIN_HASH(const PLAIN_BYTE* data, PLAIN_WORD_DOUBLE length);

/* Reserves <number> bytes of memory in <segment>. */
PLAIN_WORD_DOUBLE PLAIN_RESERVE(PLAIN_WORD_DOUBLE number, struct PLAIN_SEGMENT* segment);

/* Allocates <destination> bytes of memory. Note that <data> will be reallocated if <source> > 0, released if <destination> = 0. */
void* PLAIN_RESIZE(void* data, PLAIN_WORD_DOUBLE destination, PLAIN_WORD_DOUBLE source);

/* C++. */
#ifdef __cplusplus
}
#endif
