/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     10/05/2013,
 * Revision 12/03/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
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

/* Compiles and evaluates <segment>. Notice: <object> will be modified, <segment> will be copied. */ 
MOCOSEL_WORD_DOUBLE MOCOSEL_RUN(void* context, MOCOSEL_WORD_DOUBLE flag, const struct MOCOSEL_MANIFEST* manifest, struct MOCOSEL_OBJECT* object, const struct MOCOSEL_SEGMENT* segment);

/* Returns Mocosel version. Other version-specific information will be stored in <manifest> if given. */
MOCOSEL_WORD_DOUBLE MOCOSEL_VERSION(struct MOCOSEL_MANIFEST* manifest);

/* C++. */
#ifdef __cplusplus
}
#endif
