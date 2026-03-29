/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     10/05/2013.
 * Revision 03/28/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Runtime — the complete runtime API. Include this to embed Plain.
 * For the bare parsing layer only, include <Plain/Parser.h> instead.
 */

#pragma once

#include "Parser.h"
#include "Runtime/Host/Environment.h"
#include "Runtime/Frame.h"
#include "Runtime/Context.h"

typedef struct PLAIN_CONTEXT      PLAIN_CONTEXT;
typedef struct PLAIN_ENVIRONMENT  PLAIN_ENVIRONMENT;
typedef struct PLAIN_LIST         PLAIN_LIST;
typedef struct PLAIN_SEGMENT      PLAIN_SEGMENT;
typedef struct PLAIN_VALUE        PLAIN_VALUE;

/* Compiles and evaluates <source>. */
PLAIN_WORD_DOUBLE PLAIN_EVALUATE(struct PLAIN_CONTEXT* context, PLAIN_SUBROUTINE resolver, const PLAIN_BYTE* source, PLAIN_DELEGATE tracker, PLAIN_VALUE* value);

/* Returns library version. Other version-specific information will be stored in <environment>. */
PLAIN_WORD_DOUBLE PLAIN_VERSION(struct PLAIN_ENVIRONMENT* environment);
