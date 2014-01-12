/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/12/2013,
 * Revision 01/12/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#pragma once

/* C++. */
#ifdef __cplusplus
extern "C" {
#endif

#include "Mocosel.h"

/* Generates a prototype of an arbitrary subroutine. */
#define MOCOSEL_PROTOTYPE(NAME) MOCOSEL_WORD_DOUBLE NAME(void* context, struct MOCOSEL_LIST* node, struct MOCOSEL_SEGMENT* registry, struct MOCOSEL_VALUE* value)

/* Exports <symbol> to <registry>. */
MOCOSEL_WORD_DOUBLE MOCOSEL_EXPORT(const MOCOSEL_GLYPH* MOCOSEL_RESTRICT name, struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT registry, MOCOSEL_SUBROUTINE symbol);

/* C++. */
#ifdef __cplusplus
}
#endif
