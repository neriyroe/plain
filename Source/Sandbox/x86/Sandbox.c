/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/11/2013,
 * Revision 11/15/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include "Sandbox.h"

/* Intrinsic. */
MOCOSEL_INLINE MOCOSEL_PROTOTYPE(TYPE) {
    MOCOSEL_WORD_DOUBLE index = 0;
    MOCOSEL_WORD_DOUBLE length = MOCOSEL_MEASURE(NODE);
    for(; index < length; index++) {
        struct MOCOSEL_VALUE* value = (struct MOCOSEL_VALUE*)MOCOSEL_ARGUMENT(NODE, index);
        /* MOCOSEL_TYPE_BOOLEAN. */
        if(value->type == MOCOSEL_TYPE_BOOLEAN) {
            if(value->data) {
                PLAIN_TYPE("<%s>", (struct PLAIN_SESSION*)CONTEXT, "yes");
            } else {
                PLAIN_TYPE("<%s>", (struct PLAIN_SESSION*)CONTEXT, "no");
            }
        /* MOCOSEL_TYPE_KEYWORD. */
        } else if(value->type == MOCOSEL_TYPE_KEYWORD) {
            PLAIN_TYPE("<%s>", (struct PLAIN_SESSION*)CONTEXT, (const char*)value->data);
        /* MOCOSEL_TYPE_INTEGER. */
        } else if(value->type == MOCOSEL_TYPE_INTEGER) {
            PLAIN_TYPE("%d", (struct PLAIN_SESSION*)CONTEXT, *(int*)value->data);
        /* MOCOSEL_TYPE_NIL. */
        } else if(value->type == MOCOSEL_TYPE_NIL) {
            PLAIN_TYPE("<%s>", (struct PLAIN_SESSION*)CONTEXT, "<none>");
        /* MOCOSEL_TYPE_LIST. */
        } else if(value->type == MOCOSEL_TYPE_LIST) {
            PLAIN_TYPE("<%s>", (struct PLAIN_SESSION*)CONTEXT, "<List>");
        /* MOCOSEL_TYPE_REAL. */
        } else if(value->type == MOCOSEL_TYPE_REAL) {
            PLAIN_TYPE("%f", (struct PLAIN_SESSION*)CONTEXT, *(float*)value->data);
        /* MOCOSEL_TYPE_STRING. */
        } else if(value->type == MOCOSEL_TYPE_STRING) {
            PLAIN_TYPE("%s", (struct PLAIN_SESSION*)CONTEXT, (const char*)value->data);
        /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
        } else {
            return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
        }
    }
    PLAIN_TYPE("\n", (struct PLAIN_SESSION*)CONTEXT);
    return 0;
}

int main(int count, const char* layout[]) {
    struct PLAIN_SESSION session;
    if(PLAIN_START(&session) != 0) {
        return 0;
    }
    if(count > 1) {
        PLAIN_LOAD(layout[1], &session.program, &session);
        if(session.program.segment.data.from == NULL) {
            return 0;
        }
        /* Compile. */
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_RUN(&session, MOCOSEL_SEGMENT_COMPILE | MOCOSEL_SEGMENT_RETAIN, &session.manifest, &session.program, &session.program.segment.data);
        if(error == 0) {
            /* Library. */
            error = MOCOSEL_EXPORT("type", &session.program.registry.data, &TYPE);
            if(error != 0) {
                PLAIN_TYPE("Warning! I/O might not work.\n", &session, error);
            }
            /* Execute. */
            error = MOCOSEL_RUN(&session, MOCOSEL_SEGMENT_EXECUTE, &session.manifest, &session.program, &session.program.segment.data);
            if(error != 0) {
                PLAIN_TYPE("Failed executing %s: code %d.\n", &session, layout[1], error);
            }
        } else {
            PLAIN_TYPE("Failed compiling %s: code %d.\n", &session, layout[1], error);
        }
    } else {
        PLAIN_TYPE("%s", &session, "Usage: plain <source>.\n");
    }
    PLAIN_STOP(&session);
    return 0;
}
