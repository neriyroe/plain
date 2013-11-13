/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/13/2013,
 * Revision 11/13/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include "Sandbox.h"

MOCOSEL_WORD_DOUBLE PLAIN_START(struct PLAIN_SESSION* session) {
    if(session == NULL) {
        return MOCOSEL_ERROR_SYSTEM;
    }
    memset(session, 0, sizeof(struct PLAIN_SESSION));
    if(MOCOSEL_VERSION(&session->manifest) == 0) {
        PLAIN_TYPE("%s\n", session, "Warning: Plain might not function properly on this platform or operating system.");
    }
    return 0;
}

void PLAIN_STOP(struct PLAIN_SESSION* session) {
    if(session == NULL) {
        return;
    }
    MOCOSEL_FINALIZE(&session->program);
}

void PLAIN_TYPE(const char* __restrict format, struct PLAIN_SESSION* __restrict session, ...) {
    MOCOSEL_ASSERT(format != NULL);
    MOCOSEL_ASSERT(session != NULL);
    if(format == NULL || session == NULL) {
        return;
    }
    va_list layout;
    va_start(layout, session);
    vprintf(format, layout);
    va_end(layout);
}
