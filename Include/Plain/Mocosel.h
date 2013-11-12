/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     02/23/2013,
 * Revision 11/12/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#pragma once

/* C++. */
#ifdef __cplusplus
extern "C" {
#endif

#define MOCOSEL_FREE(POINTER) MOCOSEL_RESIZE(POINTER, 0, 0)
#define MOCOSEL_MAXIMUM(LEFT, RIGHT) ((LEFT) > (RIGHT)? (LEFT): (RIGHT))
#define MOCOSEL_MINIMUM(left, right) ((LEFT) < (RIGHT)? (LEFT): (RIGHT))
#define MOCOSEL_NONE(NAME) struct MOCOSEL_VALUE NAME = {NULL, 0, MOCOSEL_TYPE_NIL}
#define MOCOSEL_NO(NAME) struct MOCOSEL_VALUE NAME = {NULL, 0, MOCOSEL_TYPE_BOOLEAN}
#define MOCOSEL_PAIR(LEFT, RIGHT, TYPE) struct TYPE {LEFT first; RIGHT second;}
#define MOCOSEL_YES(NAME) struct MOCOSEL_VALUE NAME = {(MOCOSEL_BYTE*)0xFF, 0, MOCOSEL_TYPE_BOOLEAN}

#include "System/Platform.h"
#include "Shared/Error.h"
#include "Shared/Segment.h"
#include "Shared/Processing/Type.h"
#include "Shared/Processing/List.h"
#include "Shared/Processing/Value.h"
#include "Runtime/Subroutine.h"
#include "Runtime/Statement.h"

/* Return argument of <NODE> at <POSITION>. */
#define MOCOSEL_ARGUMENT(NODE, POSITION) ((struct MOCOSEL_VALUE*)&(NODE)->layout.from[sizeof(struct MOCOSEL_VALUE) * (POSITION)])

/* Returns number of arguments of <NODE>. */
#define MOCOSEL_MEASURE(NODE) (((NODE)->layout.to - (NODE)->layout.from) / sizeof(struct MOCOSEL_VALUE))

/* Binds <NAME> to <SYMBOL>, storing a statement in <REGISTRY>. Notice: <SYMBOL> has to be declared. */
#define MOCOSEL_SYNTHESIZE(NAME, REGISTRY, SYMBOL) {\
    /* Name. */ \
    const char SYMBOL##_STRING[] = NAME; \
    /* MOCOSEL_STATEMENT. */ \
    struct MOCOSEL_STATEMENT SYMBOL##_STATEMENT; \
    /* Keyword. */ \
    SYMBOL##_STATEMENT.first.from = (MOCOSEL_BYTE*)SYMBOL##_STRING; \
    SYMBOL##_STATEMENT.first.to = SYMBOL##_STATEMENT.first.from + strlen(SYMBOL##_STRING); \
    /* Subroutine. */ \
    SYMBOL##_STATEMENT.second = &SYMBOL; \
    /* Registry. */ \
    MOCOSEL_REGISTER(REGISTRY, &SYMBOL##_STATEMENT); \
}

/* Appends <source> to <destination>. Notice: <destination> will be reallocated. */
MOCOSEL_WORD_DOUBLE MOCOSEL_CONCAT(struct MOCOSEL_SEGMENT* __restrict destination, struct MOCOSEL_SEGMENT* __restrict source);

/* Returns 32-bit hash of <segment>. */
MOCOSEL_WORD_DOUBLE MOCOSEL_HASH(struct MOCOSEL_SEGMENT* segment);

/* Appends an argument of type <type> and length <length> to the layout of <node>. */
MOCOSEL_WORD_DOUBLE MOCOSEL_JOIN(MOCOSEL_BYTE* __restrict data, MOCOSEL_WORD_DOUBLE length, struct MOCOSEL_LIST* __restrict node, MOCOSEL_WORD_DOUBLE type);

/* Returns statement matching <keyword> in registry, or NULL otherwise. */
struct MOCOSEL_STATEMENT* MOCOSEL_LOOKUP(struct MOCOSEL_SEGMENT* __restrict keyword, struct MOCOSEL_SEGMENT* __restrict registry);

/* Frees all memory occupied by <node>, resulting in a nil list. */
void MOCOSEL_PURGE(struct MOCOSEL_LIST* node);

/* Allocates <destination> bytes of memory. Notice: <data> will be reallocated if <source> > 0, released if destination = 0. */
void* MOCOSEL_RESIZE(void* data, MOCOSEL_WORD_DOUBLE destination, MOCOSEL_WORD_DOUBLE source);

/* Adds <statement> to <registry>. */
MOCOSEL_WORD_DOUBLE MOCOSEL_REGISTER(struct MOCOSEL_SEGMENT* __restrict registry, struct MOCOSEL_STATEMENT* __restrict statement);

/* Parses <segment> to <node>. Notice: <parent> can be NULL. */
MOCOSEL_WORD_DOUBLE MOCOSEL_TOKENIZE(struct MOCOSEL_LIST* __restrict node, struct MOCOSEL_LIST* __restrict parent, struct MOCOSEL_SEGMENT* __restrict pattern, struct MOCOSEL_SEGMENT* __restrict segment);

/* Removes <keyword> from <registry>. */
MOCOSEL_WORD_DOUBLE MOCOSEL_UNREGISTER(struct MOCOSEL_SEGMENT* __restrict keyword, struct MOCOSEL_SEGMENT* __restrict registry);

/* Evaluates <node>, returning value. Notice: <value> can be NULL, procedures do not return values. */
MOCOSEL_WORD_DOUBLE MOCOSEL_WALK(struct MOCOSEL_LIST* __restrict node, struct MOCOSEL_SEGMENT* __restrict registry, struct MOCOSEL_VALUE* __restrict value);

/* C++. */
#ifdef __cplusplus
}
#endif
