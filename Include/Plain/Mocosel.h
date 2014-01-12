/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     02/23/2013,
 * Revision 01/12/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#pragma once

/* C++. */
#ifdef __cplusplus
extern "C" {
#endif

#define MOCOSEL_MAXIMUM(LEFT, RIGHT) ((LEFT) > (RIGHT)? (LEFT): (RIGHT))
#define MOCOSEL_MINIMUM(LEFT, RIGHT) ((LEFT) < (RIGHT)? (LEFT): (RIGHT))
#define MOCOSEL_PAIR(LEFT, RIGHT, TYPE) struct TYPE {LEFT first; RIGHT second;}

#include "System/Platform.h"
#include "Shared/Error.h"
#include "Shared/Segment.h"
#include "Shared/Processing/Type.h"
#include "Shared/Processing/List.h"
#include "Shared/Processing/Value.h"
#include "Runtime/Context.h"
#include "Runtime/Lookup.h"
#include "Runtime/Subroutine.h"

/* Returns argument at <position>. */
MOCOSEL_INLINE struct MOCOSEL_VALUE* MOCOSEL_ARGUMENT(struct MOCOSEL_LIST* node, MOCOSEL_WORD_DOUBLE position) {
    MOCOSEL_ASSERT(node != NULL);
    if(node == NULL) {
        return NULL;
    }
    MOCOSEL_WORD_DOUBLE length = node->layout.to - node->layout.from;
    MOCOSEL_WORD_DOUBLE offset = sizeof(struct MOCOSEL_VALUE) * position;
    if(offset >= length) {
        return NULL;
    }
    return (struct MOCOSEL_VALUE*)&node->layout.from[offset];
}

/* Appends <source> to <destination>. Note that <destination> will be reallocated. */
MOCOSEL_WORD_DOUBLE MOCOSEL_CONCAT(struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT destination, const struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT source);

/* Returns 32-bit hash of <data>. */
MOCOSEL_WORD_DOUBLE MOCOSEL_HASH(const MOCOSEL_BYTE* data, MOCOSEL_WORD_DOUBLE length);

/* Returns number of arguments. */
MOCOSEL_INLINE MOCOSEL_WORD_DOUBLE MOCOSEL_MEASURE(const struct MOCOSEL_LIST* node) {
    MOCOSEL_ASSERT(node != NULL);
    if(node == NULL) {
        return 0;
    }
    return (node->layout.to - node->layout.from) / sizeof(struct MOCOSEL_VALUE);
}

/* Frees all memory occupied by <node>. */
void MOCOSEL_PURGE(struct MOCOSEL_LIST* node);

/* Reserves <number> bytes of memory in <segment>. */
MOCOSEL_WORD_DOUBLE MOCOSEL_RESERVE(MOCOSEL_WORD_DOUBLE number, struct MOCOSEL_SEGMENT* segment);

/* Allocates <destination> bytes of memory. Note that <data> will be reallocated if <source> > 0, released if destination = 0. */
void* MOCOSEL_RESIZE(void* data, MOCOSEL_WORD_DOUBLE destination, MOCOSEL_WORD_DOUBLE source);

/* Compiles <segment> to nodes. Note that only <parent> can be NULL. */
MOCOSEL_WORD_DOUBLE MOCOSEL_TOKENIZE(struct MOCOSEL_LIST* MOCOSEL_RESTRICT node, struct MOCOSEL_LIST* MOCOSEL_RESTRICT parent, const MOCOSEL_BYTE* MOCOSEL_RESTRICT pattern, struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT segment);

/* Evaluates <node>, returning a value. Note that procedures do not return values. */
MOCOSEL_WORD_DOUBLE MOCOSEL_WALK(MOCOSEL_CONTEXT* context, MOCOSEL_LOOKUP function, struct MOCOSEL_LIST* node, struct MOCOSEL_VALUE* value);

/* C++. */
#ifdef __cplusplus
}
#endif
