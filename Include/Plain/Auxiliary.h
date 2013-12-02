/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/12/2013,
 * Revision 12/02/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#pragma once

/* C++. */
#ifdef __cplusplus
extern "C" {
#endif

#include "Mocosel.h"

/* Generates a prototype of an arbitrary subroutine. */
#define MOCOSEL_PROTOTYPE(NAME) MOCOSEL_WORD_DOUBLE NAME(void* MOCOSEL_RESTRICT CONTEXT, struct MOCOSEL_LIST* MOCOSEL_RESTRICT NODE, struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT REGISTRY, struct MOCOSEL_VALUE* MOCOSEL_RESTRICT VALUE)

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

/* Exports <symbol> to <registry>, forming a statement. Notice: <symbol> has to be declared, pass NULL to <symbol> to remove the statement. */
MOCOSEL_WORD_DOUBLE MOCOSEL_EXPORT(const MOCOSEL_BYTE* MOCOSEL_RESTRICT name, struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT registry, MOCOSEL_SUBROUTINE symbol);

/* Returns number of arguments of <node>. */
MOCOSEL_INLINE MOCOSEL_WORD_DOUBLE MOCOSEL_MEASURE(const struct MOCOSEL_LIST* node) {
    MOCOSEL_ASSERT(node != NULL);
    if(node == NULL) {
        return 0;
    }
    return (node->layout.to - node->layout.from) / sizeof(struct MOCOSEL_VALUE);
}

/* C++. */
#ifdef __cplusplus
}
#endif
