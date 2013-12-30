/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/12/2013,
 * Revision 12/30/2013,
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
#define MOCOSEL_PROTOTYPE(NAME) MOCOSEL_WORD_DOUBLE NAME(void* CONTEXT, struct MOCOSEL_LIST* MOCOSEL_RESTRICT NODE, struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT REGISTRY, struct MOCOSEL_VALUE* VALUE)

/* Exports <symbol> to <registry>, forming a statement. Notice: <symbol> has to be declared, pass NULL to <symbol> to remove the statement. */
MOCOSEL_WORD_DOUBLE MOCOSEL_EXPORT(const MOCOSEL_BYTE* MOCOSEL_RESTRICT name, struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT registry, MOCOSEL_SUBROUTINE symbol);

/* C++. */
#ifdef __cplusplus
}
#endif
