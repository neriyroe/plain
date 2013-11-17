/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/13/2013,
 * Revision 11/17/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include "Complain.h"

MOCOSEL_WORD_DOUBLE COMPLAIN_START(struct COMPLAIN_SESSION* session) {
    MOCOSEL_ASSERT(session != NULL);
    /* MOCOSEL_ERROR_SYSTEM. */
    if(session == NULL) {
        return MOCOSEL_ERROR_SYSTEM;
    }
    memset(session, 0, sizeof(struct COMPLAIN_SESSION));
    if(MOCOSEL_VERSION(&session->manifest) == 0) {
        COMPLAIN_WRITE("%s\n", session, "Warning: Complain might not operate properly on this platform or operating system.");
    }
    return 0;
}

void COMPLAIN_STOP(struct COMPLAIN_SESSION* session) {
    if(session == NULL) {
        return;
    }
    MOCOSEL_FINALIZE(&session->program);
}

void COMPLAIN_WRITE(const char* MOCOSEL_RESTRICT format, struct COMPLAIN_SESSION* MOCOSEL_RESTRICT session, ...) {
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
