/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     11/01/2014.
 * Revision 03/22/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 */

#include <stdio.h>
#include <Plain/VM.h>

void report(void* context, const PLAIN_BYTE* data, PLAIN_WORD_DOUBLE length, PLAIN_WORD_DOUBLE type);
PLAIN_WORD_DOUBLE command_handler(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value);
int evaluate(void* context, const char* source);
int prompt(void* context, const char* identifier, int (*listener)(void*, const char*));

int main() {
    struct PLAIN_CONTEXT context = {0};
    if(PLAIN_VERSION(&context.environment) == 0) {
        printf("Warning! Your platform is not supported, some functionality might be unavailable.\n");
    }
    context.frame = PLAIN_FRAME_CREATE(NULL);
    context.tracker = &report;
    context.handler = &command_handler;
    if(context.frame == NULL) {
        printf("Failed to allocate memory.\n");
        return 1;
    }
    PLAIN_CONTEXT_INIT(&context);
    int result = prompt(&context, "do: ", &evaluate);
    PLAIN_FRAME_DESTROY(context.frame);
    return result;
}
