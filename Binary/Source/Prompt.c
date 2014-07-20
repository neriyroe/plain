/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     07/20/2014,
 * Revision 07/20/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <Plain/VM.h>

MOCOSEL_VALUE* invoke(MOCOSEL_CONTEXT* context, const MOCOSEL_SEGMENT* segment) {
    MOCOSEL_ASSERT(segment != NULL);
    if(segment == NULL) {
        return NULL;
    }
    printf("\tInvoking %.*s...\n", (int)(segment->to - segment->from), (const char*)segment->from);
    return NULL;
}

void error(MOCOSEL_WORD_DOUBLE error) {
    if(error == 0) {
        return; /* No error. */
    }
    const char* category = "Unknown";
    if(error & MOCOSEL_ERROR_RUNTIME) {
        category = "Runtime";
    } else if(error & MOCOSEL_ERROR_SYNTAX) {
        category = "Syntax";
    } else if(error & MOCOSEL_ERROR_SYSTEM) {
        category = "System";
    }
    printf("\t%s error %X.\n", category, error);
}

void evaluate(void* context, const MOCOSEL_BYTE* source) {
    MOCOSEL_ASSERT(context != NULL);
    MOCOSEL_ASSERT(source != NULL);
    if(context == NULL || source == NULL) {
        return;
    }
    MOCOSEL_CONSTRUCT(MOCOSEL_OBJECT, object);
    if(*source != '\0') {
        error(
             MOCOSEL_RUN(NULL,
                         (MOCOSEL_ENVIRONMENT*)context,
                         MOCOSEL_SEGMENT_COMPILE | MOCOSEL_SEGMENT_EXECUTE,
                         &invoke,
                         &object,
                         source));
    }
    MOCOSEL_FINALIZE(&object);
}

enum {
    PROMPT_CONTINUE = 0,
    PROMPT_BREAK    = 1
};

MOCOSEL_WORD_DOUBLE prompt(void* context, void (*receiver) (void*, const MOCOSEL_BYTE*)) {
    char* line = readline("local: ");
    if (line == NULL) {
        return PROMPT_BREAK;
    }
    if (receiver != NULL) {
        receiver (context, (const MOCOSEL_BYTE*)line);
    }
    free(line);
    return PROMPT_CONTINUE;
}

int main() {
    MOCOSEL_ENVIRONMENT environment;
    MOCOSEL_VERSION(&environment);
    printf("You are running Plain %d.%d (Mocosel %s).\n", MOCOSEL_API / 10, MOCOSEL_API % 10, environment.meta.version);
    printf("Please visit http://mocosel.org to retrieve updates.\n\n");
    while(prompt(&environment, &evaluate) == PROMPT_CONTINUE) {
    }
    return 0;
}
