/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/11/2013,
 * Revision 11/11/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/VM.h>
#include <Plain/Auxiliary.h>

/* Reads all data from an external resource to <segment>. */
void PLAIN_FETCH(const char* __restrict identifier, struct MOCOSEL_SEGMENT* __restrict segment);
