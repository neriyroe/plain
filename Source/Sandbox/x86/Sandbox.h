/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/11/2013,
 * Revision 11/16/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <stdio.h>
#include <Plain/VM.h>
#include <Plain/Auxiliary.h>

struct PLAIN_SESSION {
    struct MOCOSEL_MANIFEST manifest;
    struct MOCOSEL_OBJECT   program;
};

/* Loads <object> from an external resource. */
MOCOSEL_WORD_DOUBLE PLAIN_LOAD(const char* MOCOSEL_RESTRICT identifier, struct MOCOSEL_OBJECT* MOCOSEL_RESTRICT object, struct PLAIN_SESSION* MOCOSEL_RESTRICT session);

/* Starts a new session. */
MOCOSEL_WORD_DOUBLE PLAIN_START(struct PLAIN_SESSION* session);

/* Stops the session. */
void PLAIN_STOP(struct PLAIN_SESSION* session);

/* Prints formatted string of arguments to the session buffer. */
void PLAIN_WRITE(const char* MOCOSEL_RESTRICT format, struct PLAIN_SESSION* MOCOSEL_RESTRICT session, ...);
