/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/11/2013,
 * Revision 11/17/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include "Complain.h"

/* For use within the translation unit. */
MOCOSEL_PROTOTYPE(COMPLAIN_OUT) {
    MOCOSEL_WORD_DOUBLE index = 0;
    MOCOSEL_WORD_DOUBLE length = MOCOSEL_MEASURE(NODE);
    for(; index < length; index++) {
        struct MOCOSEL_VALUE* value = MOCOSEL_ARGUMENT(NODE, index);
        /* MOCOSEL_TYPE_BOOLEAN. */
        if(value->type == MOCOSEL_TYPE_BOOLEAN) {
            if(value->data == NULL) {
                COMPLAIN_WRITE("<no>", (struct COMPLAIN_SESSION*)CONTEXT);
            } else {
                COMPLAIN_WRITE("<yes>", (struct COMPLAIN_SESSION*)CONTEXT);
            }
        /* MOCOSEL_TYPE_INTEGER. */
        } else if(value->type == MOCOSEL_TYPE_INTEGER) {
            COMPLAIN_WRITE("%d", (struct COMPLAIN_SESSION*)CONTEXT, *(int*)value->data);
        /* MOCOSEL_TYPE_KEYWORD. */
        } else if(value->type == MOCOSEL_TYPE_KEYWORD) {
            COMPLAIN_WRITE("<%s>", (struct COMPLAIN_SESSION*)CONTEXT, (const char*)value->data);
        /* MOCOSEL_TYPE_LIST. */
        } else if(value->type == MOCOSEL_TYPE_LIST) {
            COMPLAIN_WRITE("<List>", (struct COMPLAIN_SESSION*)CONTEXT);
        /* MOCOSEL_TYPE_NIL. */
        } else if(value->type == MOCOSEL_TYPE_NIL) {
            COMPLAIN_WRITE("<none>", (struct COMPLAIN_SESSION*)CONTEXT);
        /* MOCOSEL_TYPE_REAL. */
        } else if(value->type == MOCOSEL_TYPE_REAL) {
            COMPLAIN_WRITE("%f", (struct COMPLAIN_SESSION*)CONTEXT, *(float*)value->data);
        /* MOCOSEL_TYPE_STRING. */
        } else if(value->type == MOCOSEL_TYPE_STRING) {
            COMPLAIN_WRITE("%s", (struct COMPLAIN_SESSION*)CONTEXT, (const char*)value->data);
        }
    }
    COMPLAIN_WRITE("\n", (struct COMPLAIN_SESSION*)CONTEXT);
    return 0;
}

int main(int count, const char* layout[]) {
    struct COMPLAIN_SESSION session;
    if(COMPLAIN_START(&session) != 0) {
        return 0;
    }
    if(count > 1) {
        MOCOSEL_WORD_DOUBLE error = COMPLAIN_LOAD(layout[1], &session.program, &session);
        if(error == 0) {
            error = MOCOSEL_EXPORT("out", &session.program.registry.data, &COMPLAIN_OUT);
            if(error != 0) {
                COMPLAIN_WRITE("Warning: could not resolve `%s`.\n", &session, "out");
            }
            error = MOCOSEL_RUN(&session, MOCOSEL_SEGMENT_EXECUTE, &session.manifest, &session.program, NULL);
            /* MOCOSEL_ERROR_SYSTEM. */
            if(error & MOCOSEL_ERROR_SYSTEM) {
                COMPLAIN_WRITE("System error %d occured while executing %s.\n", &session, error, layout[1]);
            /* MOCOSEL_ERROR_RUNTIME. */
            } else if(error & MOCOSEL_ERROR_RUNTIME) {
                COMPLAIN_WRITE("Runtime error %d occured while executing %s.\n", &session, error, layout[1]);
            } else if(error != 0) {
                COMPLAIN_WRITE("Error %d occured while executing %s.\n", &session, error, layout[1]);
            }
        }
    } else {
        COMPLAIN_WRITE("%s", &session, "Usage: Complain <source>.\n");
    }
    COMPLAIN_STOP(&session);
    return 0;
}
