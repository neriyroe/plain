/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     05/25/2014.
 * Revision 09/01/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 *
 * CL — core library primitives.
 *
 * Pulls in the platform layer and segment type, then declares the
 * low-level memory and buffer operations that everything else is built on.
 */

#pragma once

#include "System/Platform.h"
#include "Shared/Segment.h"

/* Appends <length> bytes from <source> to the end of the <destination>
 * segment, growing the underlying buffer via PLAIN_RESIZE. */
PLAIN_WORD_DOUBLE PLAIN_CONCATENATE(struct PLAIN_SEGMENT* destination, PLAIN_WORD_DOUBLE length, const PLAIN_BYTE* source);

/* Computes a 32-bit FNV-1a hash of the first <length> bytes of <data>.
 * Used by the tokenizer to identify reserved literals without strcmp. */
PLAIN_WORD_DOUBLE PLAIN_HASH(const PLAIN_BYTE* data, PLAIN_WORD_DOUBLE length);

/* Grows <segment> by <number> bytes using PLAIN_RESIZE.  Existing content
 * is preserved; the from/to pointers are updated to the new allocation. */
PLAIN_WORD_DOUBLE PLAIN_RESERVE(PLAIN_WORD_DOUBLE number, struct PLAIN_SEGMENT* segment);

/* The single memory management primitive.  All allocations, reallocations,
 * and frees in the runtime go through this function.
 *   PLAIN_RESIZE(NULL, size, 0)    — allocate (like malloc).
 *   PLAIN_RESIZE(ptr, newsize, _)  — reallocate (like realloc).
 *   PLAIN_RESIZE(ptr, 0, _)        — free.
 *   PLAIN_RESIZE(ptr, size, size)  — no-op (sizes match). */
void* PLAIN_RESIZE(void* data, PLAIN_WORD_DOUBLE destination, PLAIN_WORD_DOUBLE source);
