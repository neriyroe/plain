/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     10/05/2013.
 * Revision 09/01/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

#pragma once

/* C++. */
#ifdef __cplusplus
extern "C" {
#endif

#include "Plain.h"
#include "Framework/Host/Environment.h"
#include "Framework/Object.h"

typedef struct PLAIN_ENVIRONMENT  PLAIN_ENVIRONMENT;
typedef struct PLAIN_LIST         PLAIN_LIST;
typedef struct PLAIN_OBJECT       PLAIN_OBJECT;
typedef struct PLAIN_SEGMENT      PLAIN_SEGMENT;
typedef struct PLAIN_VALUE        PLAIN_VALUE;

/* Compiles and evaluates <source>. */
PLAIN_WORD_DOUBLE PLAIN_EVALUATE(PLAIN_ENVIRONMENT* environment, PLAIN_SUBROUTINE function, const PLAIN_BYTE* source, PLAIN_DELEGATE tracker, PLAIN_VALUE* value);

/* Returns library version. Other version-specific information will be stored in <environment>. */
PLAIN_WORD_DOUBLE PLAIN_VERSION(PLAIN_ENVIRONMENT* environment);

/* C++. */
#ifdef __cplusplus
}
#endif
