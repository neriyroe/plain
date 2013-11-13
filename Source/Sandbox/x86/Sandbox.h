/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/11/2013,
 * Revision 11/13/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <stdio.h>
#include <Plain/VM.h>
#include <Plain/Auxiliary.h>

struct PLAIN_SESSION {
    struct MOCOSEL_MANIFEST manifest;
    struct MOCOSEL_OBJECT program;
};

/* Reads all data from an external resource to <segment>. */
void PLAIN_LOAD(const char* __restrict identifier, struct MOCOSEL_OBJECT* __restrict object);

/* Starts a new session. */
void PLAIN_START(struct PLAIN_SESSION* session);

/* Stops the session. */
void PLAIN_STOP(struct PLAIN_SESSION* session);
