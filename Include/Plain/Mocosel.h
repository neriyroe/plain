/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     02/23/2013,
 * Revision 10/16/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#pragma once

/* C++. */
#ifdef __cplusplus
extern "C" {
#endif

/* Auxiliary. */
#define MOCOSEL_FREE(pointer) MOCOSEL_RESIZE(pointer, 0, 0)
#define MOCOSEL_MAXIMUM(left, right) ((left) > (right)? (left): (right))
#define MOCOSEL_MINIMUM(left, right) ((left) < (right)? (left): (right))
#define MOCOSEL_NONE(name) struct MOCOSEL_VALUE name = {NULL, 0, MOCOSEL_TYPE_NIL}
#define MOCOSEL_NO(name) struct MOCOSEL_VALUE name = {NULL, 0, MOCOSEL_TYPE_BOOLEAN}
#define MOCOSEL_PAIR(left, right, type) struct type {left first; right second;}
#define MOCOSEL_YES(name) struct MOCOSEL_VALUE name = {(MOCOSEL_BYTE*)0xFF, 0, MOCOSEL_TYPE_BOOLEAN}

/* Configuration. */
#include "System/Platform.h"

/* Shared. */
#include "Shared/Error.h"
#include "Shared/Segment.h"
#include "Shared/Concat.h"

/* System. */
#include "System/Memory.h"

/* Algorithm. */
#include "Shared/Algorithm/Hash.h"

/* Processing. */
#include "Shared/Processing/Type.h"
#include "Shared/Processing/List.h"
#include "Shared/Processing/Value.h"
#include "Shared/Processing/Tokenizer.h"
#include "Shared/Processing/Join.h"
#include "Shared/Processing/Purge.h"

/* Runtime. */
#include "Runtime/Subroutine.h"
#include "Runtime/Statement.h"
#include "Runtime/Lookup.h"
#include "Runtime/Register.h"
#include "Runtime/Unregister.h"
#include "Runtime/Walk.h"
#include "Runtime/Yield.h"

/* Runtime. */
#include "Runtime/Walk.h"
#include "Runtime/Yield.h"

/* C++. */
#ifdef __cplusplus
}
#endif
