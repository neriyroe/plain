/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/11/2013,
 * Revision 11/16/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include "Sandbox.h"

/* For use within the translation unit. */
MOCOSEL_PROTOTYPE(PLAIN_OUT) {
    MOCOSEL_WORD_DOUBLE index = 0;
    MOCOSEL_WORD_DOUBLE length = MOCOSEL_MEASURE(NODE);
    for(; index < length; index++) {
        struct MOCOSEL_VALUE* value = MOCOSEL_ARGUMENT(NODE, index);
        /* MOCOSEL_TYPE_BOOLEAN. */
        if(value->type == MOCOSEL_TYPE_BOOLEAN) {
            if(value->data == NULL) {
                PLAIN_WRITE("<no>", (struct PLAIN_SESSION*)CONTEXT);
            } else {
                PLAIN_WRITE("<yes>", (struct PLAIN_SESSION*)CONTEXT);
            }
        /* MOCOSEL_TYPE_INTEGER. */
        } else if(value->type == MOCOSEL_TYPE_INTEGER) {
            PLAIN_WRITE("%d", (struct PLAIN_SESSION*)CONTEXT, *(int*)value->data);
        /* MOCOSEL_TYPE_KEYWORD. */
        } else if(value->type == MOCOSEL_TYPE_KEYWORD) {
            PLAIN_WRITE("<%s>", (struct PLAIN_SESSION*)CONTEXT, (const char*)value->data);
        /* MOCOSEL_TYPE_LIST. */
        } else if(value->type == MOCOSEL_TYPE_LIST) {
            PLAIN_WRITE("<List>", (struct PLAIN_SESSION*)CONTEXT);
        /* MOCOSEL_TYPE_NIL. */
        } else if(value->type == MOCOSEL_TYPE_NIL) {
            PLAIN_WRITE("<none>", (struct PLAIN_SESSION*)CONTEXT);
        /* MOCOSEL_TYPE_REAL. */
        } else if(value->type == MOCOSEL_TYPE_REAL) {
            PLAIN_WRITE("%f", (struct PLAIN_SESSION*)CONTEXT, *(float*)value->data);
        /* MOCOSEL_TYPE_STRING. */
        } else if(value->type == MOCOSEL_TYPE_STRING) {
            PLAIN_WRITE("%s", (struct PLAIN_SESSION*)CONTEXT, (const char*)value->data);
        }
    }
    PLAIN_WRITE("\n", (struct PLAIN_SESSION*)CONTEXT);
    return 0;
}

int main(int count, const char* layout[]) {
    struct PLAIN_SESSION session;
    if(PLAIN_START(&session) != 0) {
        return 0;
    }
    if(count > 1) {
        MOCOSEL_WORD_DOUBLE error = PLAIN_LOAD(layout[1], &session.program, &session);
        if(error == 0) {
            error = MOCOSEL_EXPORT("out", &session.program.registry.data, &PLAIN_OUT);
            if(error != 0) {
                PLAIN_WRITE("Warning: could not resolve `%s`.\n", &session, "out");
            }
            error = MOCOSEL_RUN(&session, MOCOSEL_SEGMENT_EXECUTE, &session.manifest, &session.program, NULL);
            /* MOCOSEL_ERROR_SYSTEM. */
            if(error & MOCOSEL_ERROR_SYSTEM) {
                PLAIN_WRITE("System error %d occured while executing %s.\n", &session, error, layout[1]);
            /* MOCOSEL_ERROR_RUNTIME. */
            } else if(error & MOCOSEL_ERROR_RUNTIME) {
                PLAIN_WRITE("Runtime error %d occured while executing %s.\n", &session, error, layout[1]);
            } else if(error != 0) {
                PLAIN_WRITE("Error %d occured while executing %s.\n", &session, error, layout[1]);
            }
        }
    } else {
        PLAIN_WRITE("%s", &session, "Usage: plain <source>.\n");
    }
    PLAIN_STOP(&session);
    return 0;
}
