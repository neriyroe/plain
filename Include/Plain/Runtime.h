/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     10/05/2013.
 * Revision 03/28/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Runtime — the complete runtime API.  Include this to embed Plain.
 *
 * Pulls together the parser, frame system, and context into a single
 * header.  For the bare parsing layer only, include <Plain/Parser.h>.
 *
 * Convenience typedefs are provided so downstream code can write
 * PLAIN_CONTEXT instead of struct PLAIN_CONTEXT, etc.
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

/* Top-level entry point: tokenizes <source> into a parse tree, walks it
 * through <resolver>, and frees the tree.  Returns 0 on success.
 * <tracker> receives syntax error reports (may be NULL).
 * <value> receives the result of the last expression (may be NULL). */
PLAIN_WORD_DOUBLE PLAIN_EVALUATE(struct PLAIN_CONTEXT* context, PLAIN_SUBROUTINE resolver, const PLAIN_BYTE* source, PLAIN_DELEGATE tracker, PLAIN_VALUE* value);

/* Populates <environment> with the delimiter set and version string.
 * Returns the PLAIN_API version number.  Called once during init. */
PLAIN_WORD_DOUBLE PLAIN_VERSION(struct PLAIN_ENVIRONMENT* environment);
