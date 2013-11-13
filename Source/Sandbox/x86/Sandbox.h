/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/11/2013,
 * Revision 11/12/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <stdio.h>
#include <Plain/VM.h>
#include <Plain/Auxiliary.h>

struct PLAIN_UNIT {
    struct MOCOSEL_OBJECT   object;
    struct MOCOSEL_SEGMENT  segment;
};

/* Reads all data from an external resource to <segment>. */
void PLAIN_LOAD(const char* __restrict identifier, struct PLAIN_UNIT* __restrict unit);
