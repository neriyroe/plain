/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/17/2013,
 * Revision 11/17/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#pragma once

/* C++. */
#ifdef __cplusplus
extern "C" {
#endif

#include "Mocosel.h"
#include "VM.h"

/* As in <Compact> of the Mocosel Programming Language Specification. */
MOCOSEL_PROTOTYPE(COMPACT_AND);
MOCOSEL_PROTOTYPE(COMPACT_ARRAY);
MOCOSEL_PROTOTYPE(COMPACT_DIFFERENCE);
MOCOSEL_PROTOTYPE(COMPACT_DO);
MOCOSEL_PROTOTYPE(COMPACT_IF);
MOCOSEL_PROTOTYPE(COMPACT_IS);
MOCOSEL_PROTOTYPE(COMPACT_OR);
MOCOSEL_PROTOTYPE(COMPACT_QUOTIENT);
MOCOSEL_PROTOTYPE(COMPACT_PRODUCT);
MOCOSEL_PROTOTYPE(COMPACT_REMAINDER);
MOCOSEL_PROTOTYPE(COMPACT_RETURN);
MOCOSEL_PROTOTYPE(COMPACT_SUM);

/* Exports the library to <registry>. */
MOCOSEL_WORD_DOUBLE COMPACT_EXPORT(struct MOCOSEL_SEGMENT* registry);

/* C++. */
#ifdef __cplusplus
}
#endif
