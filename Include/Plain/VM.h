/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     10/05/2013,
 * Revision 07/15/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#pragma once

/* C++. */
#ifdef __cplusplus
extern "C" {
#endif

#include "Mocosel.h"
#include "Framework/Host/Environment.h"
#include "Framework/Object.h"

typedef struct MOCOSEL_ENVIRONMENT  MOCOSEL_ENVIRONMENT;
typedef struct MOCOSEL_LIST         MOCOSEL_LIST;
typedef struct MOCOSEL_OBJECT       MOCOSEL_OBJECT;
typedef struct MOCOSEL_SEGMENT      MOCOSEL_SEGMENT;
typedef struct MOCOSEL_VALUE        MOCOSEL_VALUE;

enum {
    MOCOSEL_SEGMENT_COMPILE = 0x01,
    MOCOSEL_SEGMENT_EXECUTE = 0x02
};

/* Frees all memory occupied by <object>. */
void MOCOSEL_FINALIZE(struct MOCOSEL_OBJECT* object);

/* Compiles and evaluates <segment>. Note that <object> will be modified, <segment> will be copied. */ 
MOCOSEL_WORD_DOUBLE MOCOSEL_RUN(MOCOSEL_CONTEXT* context, MOCOSEL_ENVIRONMENT* environment, MOCOSEL_WORD_DOUBLE flag, MOCOSEL_LOOKUP function, MOCOSEL_OBJECT* object, const MOCOSEL_BYTE* source);

/* Returns Mocosel version. Other version-specific information will be stored in <environment>. */
MOCOSEL_WORD_DOUBLE MOCOSEL_VERSION(MOCOSEL_ENVIRONMENT* environment);

/* C++. */
#ifdef __cplusplus
}
#endif
