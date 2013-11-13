/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/13/2013,
 * Revision 11/13/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include "Sandbox.h"

void PLAIN_START(struct PLAIN_SESSION* session) {
    if(session == NULL) {
        return;
    }
    if(MOCOSEL_VERSION(&session->manifest) == 0) {
        printf("Warning: Plain might not function properly on this platform or operating system.\n");
    }
    memset(&session->program, 0, sizeof(struct MOCOSEL_OBJECT));
}

void PLAIN_STOP(struct PLAIN_SESSION* session) {
    if(session == NULL) {
        return;
    }
    MOCOSEL_FINALIZE(&session->program);
}
