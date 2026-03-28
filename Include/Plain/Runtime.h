/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     10/05/2013.
 * Revision 03/28/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Runtime — the complete runtime API. Include this to embed Plain.
 * For the bare parsing layer only, include <Plain/Plain.h> instead.
 */

#pragma once

/* C++. */
#ifdef __cplusplus
extern "C" {
#endif

#include "Plain.h"
#include "Runtime/Subroutine.h"
#include "Runtime/Host/Environment.h"
#include "Runtime/Object.h"
#include "Runtime/Frame.h"
#include "Runtime/Context.h"

typedef struct PLAIN_ENVIRONMENT  PLAIN_ENVIRONMENT;
typedef struct PLAIN_LIST         PLAIN_LIST;
typedef struct PLAIN_OBJECT       PLAIN_OBJECT;
typedef struct PLAIN_SEGMENT      PLAIN_SEGMENT;
typedef struct PLAIN_VALUE        PLAIN_VALUE;

/* Compiles and evaluates <source>. */
PLAIN_WORD_DOUBLE PLAIN_EVALUATE(PLAIN_ENVIRONMENT* environment, PLAIN_SUBROUTINE function, const PLAIN_BYTE* source, PLAIN_DELEGATE tracker, PLAIN_VALUE* value);

/* Evaluates the list given by <node>. Note that both <context> and <value> can be NULL. */
PLAIN_WORD_DOUBLE PLAIN_WALK(void* context, PLAIN_SUBROUTINE function, struct PLAIN_LIST* node, struct PLAIN_VALUE* value);

/* Returns library version. Other version-specific information will be stored in <environment>. */
PLAIN_WORD_DOUBLE PLAIN_VERSION(PLAIN_ENVIRONMENT* environment);

/* C++. */
#ifdef __cplusplus
}
#endif
