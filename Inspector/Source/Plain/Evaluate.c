/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     11/01/2014.
 * Revision 03/22/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 */

#include <stdio.h>
#include <Plain/VM.h>

PLAIN_WORD_DOUBLE print_native(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    PLAIN_WORD_DOUBLE i = 0;
    for(; i < arity; i++) {
        if(i > 0) putchar(' ');
        struct PLAIN_VALUE* argument = PLAIN_ARGUMENT(node, i);
        switch(argument->type) {
            case PLAIN_TYPE_STRING:  printf("%s",  (const char*)argument->data); break;
            case PLAIN_TYPE_INTEGER: printf("%d",  (int)*(PLAIN_WORD_DOUBLE*)argument->data); break;
            case PLAIN_TYPE_REAL:    printf("%g",  (double)*(PLAIN_REAL*)argument->data); break;
            case PLAIN_TYPE_BOOLEAN: printf("%s",  argument->data != NULL ? "yes" : "no"); break;
            case PLAIN_TYPE_NIL:     printf("none"); break;
            default: break;
        }
    }
    putchar('\n');
    return 0;
}

PLAIN_WORD_DOUBLE evaluate(void* raw, const char* source) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    PLAIN_WORD_DOUBLE error = PLAIN_EVALUATE(&context->environment, &PLAIN_RESOLVE, (const PLAIN_BYTE*)source, context->tracker, NULL);
    if(error == PLAIN_ERROR_SYNTAX) {
        printf("The evaluation failed due to a syntax error.\n");
    } else if(error != 0) {
        printf("The evaluation failed due to an unidentified error (0x%X).\n", error);
    }
    return 0;
}
