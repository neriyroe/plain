/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/11/2013,
 * Revision 11/13/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include "Sandbox.h"

int main(int count, const char* layout[]) {
    struct PLAIN_SESSION session;
    if(count > 1) {
        PLAIN_START(&session);
        PLAIN_LOAD(layout[1], &session.program);
        if(session.program.segment.data.from == NULL) {
            return 0;
        }
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_RUN(MOCOSEL_SEGMENT_COMPILE | MOCOSEL_SEGMENT_RETAIN, &session.manifest, &session.program, &session.program.segment.data);
        if(error != 0) {
            printf("Failed compiling %s: code %d.\n", layout[1], error);
        }
        if(error == 0) {
            error = MOCOSEL_RUN(MOCOSEL_SEGMENT_EXECUTE, &session.manifest, &session.program, NULL);
            if(error != 0) {
                printf("Failed executing %s: code %d.\n", layout[1], error);
            }
        }
        PLAIN_STOP(&session);
    } else {
        printf("Usage: plain <source>.\n");
    }
    return 0;
}
