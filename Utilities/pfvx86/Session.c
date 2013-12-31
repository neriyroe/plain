/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/13/2013,
 * Revision 11/18/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include "Easy.h"

MOCOSEL_WORD_DOUBLE EASY_START(struct EASY_SESSION* session) {
    MOCOSEL_ASSERT(session != NULL);
    /* MOCOSEL_ERROR_SYSTEM. */
    if(session == NULL) {
        return MOCOSEL_ERROR_SYSTEM;
    }
    memset(session, 0, sizeof(struct EASY_SESSION));
    if(MOCOSEL_VERSION(&session->manifest) == 0) {
        EASY_WRITE("%s\n", session, "Warning: Easy might not operate properly on this platform or operating system.");
    }
    return 0;
}

void EASY_STOP(struct EASY_SESSION* session) {
    if(session == NULL) {
        return;
    }
    MOCOSEL_FINALIZE(&session->program);
}

void EASY_WRITE(const char* MOCOSEL_RESTRICT format, struct EASY_SESSION* MOCOSEL_RESTRICT session, ...) {
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
