/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/12/2013,
 * Revision 11/12/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#pragma once

/* C++. */
#ifdef __cplusplus
extern "C" {
#endif

#include "Mocosel.h"

/* Returns argument of <node> at <position>. */
struct MOCOSEL_VALUE* MOCOSEL_ARGUMENT(struct MOCOSEL_LIST* node, MOCOSEL_WORD_DOUBLE position);

/* Exports <symbol> to <registry>, forming a statement. Notice: <symbol> has to be declared, pass NULL to <symbol> to remove the statement. */
MOCOSEL_WORD_DOUBLE MOCOSEL_EXPORT(MOCOSEL_BYTE* __restrict name, struct MOCOSEL_SEGMENT* __restrict registry, MOCOSEL_SUBROUTINE symbol);

/* Returns number of arguments of <node>. */
MOCOSEL_WORD_DOUBLE MOCOSEL_MEASURE(struct MOCOSEL_LIST* node);

/* C++. */
#ifdef __cplusplus
}
#endif
