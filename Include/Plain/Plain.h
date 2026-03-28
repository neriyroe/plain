/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     02/23/2013.
 * Revision 09/01/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

#pragma once

#define PLAIN_API 20153 /* API version. */

#include "CL.h"
#include "Shared/Error.h"
#include "Shared/Processing/Type.h"
#include "Shared/Processing/List.h"
#include "Shared/Processing/Value.h"
#include "Shared/Processing/Delegate.h"
#include "Shared/Processing/Subroutine.h"

/* Returns argument at <position>. */
PLAIN_INLINE struct PLAIN_VALUE* PLAIN_ARGUMENT(struct PLAIN_LIST* node, PLAIN_WORD_DOUBLE position) {
    PLAIN_ASSERT(node != NULL);
    if(node == NULL) {
        return NULL;
    }
    PLAIN_WORD_DOUBLE length = node->layout.to - node->layout.from;
    PLAIN_WORD_DOUBLE offset = sizeof(struct PLAIN_VALUE) * position;
    if(offset >= length) {
        return NULL;
    }
    return (struct PLAIN_VALUE*)&node->layout.from[offset];
}

/* Returns number of arguments. */
PLAIN_INLINE PLAIN_WORD_DOUBLE PLAIN_ARITY(const struct PLAIN_LIST* node) {
    PLAIN_ASSERT(node != NULL);
    if(node == NULL) {
        return 0;
    }
    return (node->layout.to - node->layout.from) / sizeof(struct PLAIN_VALUE);
}

/* Stores <data> in <value>. Note that <data> will be copied if <length> > 0. */
PLAIN_WORD_DOUBLE PLAIN_EXPORT(PLAIN_BYTE* data, PLAIN_WORD_DOUBLE length, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value);

/* Compiles <segment> to nodes and arguments. Note that only <parent> can be NULL. */
PLAIN_WORD_DOUBLE PLAIN_TOKENIZE(void* context, struct PLAIN_LIST* node, struct PLAIN_LIST* parent, const PLAIN_BYTE* delimiters, struct PLAIN_SEGMENT* segment, PLAIN_DELEGATE tracker);

/* Evaluates the list given by <node>. Note that both <context> and <value> can be NULL. */
PLAIN_WORD_DOUBLE PLAIN_WALK(void* context, PLAIN_SUBROUTINE resolver, struct PLAIN_LIST* node, struct PLAIN_VALUE* value);

/* Frees all memory occupied by <node>. */
void PLAIN_UNLINK(struct PLAIN_LIST* node);

/* Deep-copies <node> and its entire sibling/child tree into freshly allocated memory.
 * keyword and segment fields borrow pointers into the original source buffer, which
 * must remain alive for the lifetime of the copy. Returns NULL on allocation failure. */
struct PLAIN_LIST* PLAIN_LIST_COPY(const struct PLAIN_LIST* node);

