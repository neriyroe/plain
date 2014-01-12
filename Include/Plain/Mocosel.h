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
#include "Runtime/Subroutine.h"
#include "Runtime/Statement.h"

/* Returns argument of <node> at <position>. */
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

/* Appends <source> to <destination>. Notice: <destination> will be reallocated. */
MOCOSEL_WORD_DOUBLE MOCOSEL_CONCAT(struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT destination, const struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT source);

/* Empties <registry>. */
void MOCOSEL_EMPTY(struct MOCOSEL_SEGMENT* registry);

/* Returns 32-bit hash of <data>. */
MOCOSEL_WORD_DOUBLE MOCOSEL_HASH(const MOCOSEL_BYTE* data, MOCOSEL_WORD_DOUBLE length);

/* Returns statement matching <keyword> in registry, or NULL otherwise. */
struct MOCOSEL_STATEMENT* MOCOSEL_LOOKUP(const struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT keyword, struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT registry);

/* Returns number of arguments of <node>. */
MOCOSEL_INLINE MOCOSEL_WORD_DOUBLE MOCOSEL_MEASURE(const struct MOCOSEL_LIST* node) {
    MOCOSEL_ASSERT(node != NULL);
    if(node == NULL) {
        return 0;
    }
    return (node->layout.to - node->layout.from) / sizeof(struct MOCOSEL_VALUE);
}

/* Frees all memory occupied by <node>, resulting in a nil list. */
void MOCOSEL_PURGE(struct MOCOSEL_LIST* node);

/* Reserves <number> bytes of memory in <segment>. */
MOCOSEL_WORD_DOUBLE MOCOSEL_RESERVE(MOCOSEL_WORD_DOUBLE number, struct MOCOSEL_SEGMENT* segment);

/* Allocates <destination> bytes of memory. Notice: <data> will be reallocated if <source> > 0, released if destination = 0. */
void* MOCOSEL_RESIZE(void* data, MOCOSEL_WORD_DOUBLE destination, MOCOSEL_WORD_DOUBLE source);

/* Adds <statement> to <registry>. Notice: data will not be copied. */
MOCOSEL_WORD_DOUBLE MOCOSEL_REGISTER(struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT registry, struct MOCOSEL_STATEMENT* MOCOSEL_RESTRICT statement);

/* Parses <segment> to <node>. Notice: <parent> can be NULL. */
MOCOSEL_WORD_DOUBLE MOCOSEL_TOKENIZE(struct MOCOSEL_LIST* MOCOSEL_RESTRICT node, struct MOCOSEL_LIST* MOCOSEL_RESTRICT parent, const MOCOSEL_BYTE* MOCOSEL_RESTRICT pattern, struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT segment);

/* Evaluates <node>, returning value. Notice: <value> can be NULL, procedures do not return values. */
MOCOSEL_WORD_DOUBLE MOCOSEL_WALK(void* context, struct MOCOSEL_LIST* node, struct MOCOSEL_SEGMENT* registry, struct MOCOSEL_VALUE* value);

/* C++. */
#ifdef __cplusplus
}
#endif
