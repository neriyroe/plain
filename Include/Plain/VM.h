/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     10/05/2013,
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
#include "Framework/Host/Manifest.h"
#include "Framework/Object.h"

enum {
    MOCOSEL_SEGMENT_COMPILE = 0x01,
    MOCOSEL_SEGMENT_EXECUTE = 0x02
};

/* Frees all memory occupied by <object>. */
void MOCOSEL_FINALIZE(struct MOCOSEL_OBJECT* object);

/* Compiles and evaluates <segment>. Note that <object> will be modified, <segment> will be copied. */ 
MOCOSEL_WORD_DOUBLE MOCOSEL_RUN(MOCOSEL_CONTEXT* context, MOCOSEL_WORD_DOUBLE flag, MOCOSEL_LOOKUP function, const struct MOCOSEL_MANIFEST* manifest, struct MOCOSEL_OBJECT* object, const struct MOCOSEL_SEGMENT* segment);

/* Returns Mocosel version. Other version-specific information will be stored in <manifest>. */
MOCOSEL_WORD_DOUBLE MOCOSEL_VERSION(struct MOCOSEL_MANIFEST* manifest);

/* C++. */
#ifdef __cplusplus
}
#endif
