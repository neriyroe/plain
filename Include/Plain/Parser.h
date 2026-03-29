/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     02/23/2013.
 * Revision 09/01/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 *
 * Parser — tokenization and tree-walking API.
 *
 * This is the core parsing layer.  It provides:
 *   - PLAIN_TOKENIZE: recursive-descent parser that turns source text into
 *     a tree of PLAIN_LIST nodes.
 *   - PLAIN_WALK: tree walker that resolves arguments and dispatches each
 *     node to a resolver callback.
 *   - PLAIN_EXPORT: stores a typed value into a PLAIN_VALUE slot.
 *   - PLAIN_UNLINK / PLAIN_LIST_COPY: tree memory management.
 *   - PLAIN_ARGUMENT / PLAIN_ARITY: argument access helpers (inline).
 *
 * Include <Plain/Runtime.h> instead for the full runtime (frames, callables,
 * evaluation).  Include this directly only for bare parsing without execution.
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

/* Returns a pointer to the argument at <position> (zero-based) in <node>'s
 * argument array, or NULL if <position> is out of bounds. */
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

/* Returns the number of arguments stored in <node>. */
PLAIN_INLINE PLAIN_WORD_DOUBLE PLAIN_ARITY(const struct PLAIN_LIST* node) {
    PLAIN_ASSERT(node != NULL);
    if(node == NULL) {
        return 0;
    }
    return (node->layout.to - node->layout.from) / sizeof(struct PLAIN_VALUE);
}

/* Stores <data> of the given <type> into <value>.
 * If <length> > 0, the data is heap-copied (with backslash escape processing
 * for strings and keywords).  If <length> == 0, the data pointer is stored
 * directly (used for booleans, nil, and inline markers). */
PLAIN_WORD_DOUBLE PLAIN_EXPORT(PLAIN_BYTE* data, PLAIN_WORD_DOUBLE length, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value);

/* Parses the source text in <segment> into a PLAIN_LIST node tree.
 * Each node represents one statement; siblings are linked via node->node.
 * <parent> is non-NULL when parsing inside [...] (expressions) — this
 * marks the result as an inline expression rather than a deferred block.
 * <delimiters> is the set of characters that terminate keyword tokens.
 * <tracker> receives syntax error reports (may be NULL). */
PLAIN_WORD_DOUBLE PLAIN_TOKENIZE(void* context, struct PLAIN_LIST* node, struct PLAIN_LIST* parent, const PLAIN_BYTE* delimiters, struct PLAIN_SEGMENT* segment, PLAIN_DELEGATE tracker);

/* Walks the node tree: resolves arguments (sub-expressions, interpolations),
 * then dispatches each node to <resolver>.  Continues to sibling nodes via
 * node->node.  Returns 0 on success, or an error/signal code. */
PLAIN_WORD_DOUBLE PLAIN_WALK(void* context, PLAIN_SUBROUTINE resolver, struct PLAIN_LIST* node, struct PLAIN_VALUE* value);

/* Recursively frees all memory owned by <node>: argument data, child nodes,
 * and sibling nodes.  Does not free <node> itself (use PLAIN_RESIZE for that). */
void PLAIN_UNLINK(struct PLAIN_LIST* node);

/* Deep-copies <node> and its entire sibling/child tree into freshly allocated
 * memory.  Keyword and segment fields borrow pointers into the original source
 * buffer, which must remain alive for the lifetime of the copy.
 * Returns NULL on allocation failure. */
struct PLAIN_LIST* PLAIN_LIST_COPY(const struct PLAIN_LIST* node);
