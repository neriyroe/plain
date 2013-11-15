/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/11/2013,
 * Revision 11/15/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include "Sandbox.h"

int main(int count, const char* layout[]) {
    struct PLAIN_SESSION session;
    if(PLAIN_START(&session) != 0) {
        return 0;
    }
    if(count > 1) {
        // FIX ME!
    } else {
        PLAIN_TYPE("%s", &session, "Usage: plain <source>.\n");
    }
    PLAIN_STOP(&session);
    return 0;
}
