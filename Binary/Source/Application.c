/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     11/01/2014.
 * Revision 08/30/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

#include <stdlib.h>
#include <stdio.h>
#include <Plain/VM.h>

/* Evaluate will return a proper value, the application will be terminated. */
int evaluate(void* context, const char* source);

/* Prompt will return a proper value. */
int prompt(void* context, const char* identifier, int (*listener) (void*, const char*));

int main() {
    PLAIN_ENVIRONMENT environment;
    if(PLAIN_VERSION(&environment) == 0) {
        printf("Warning! Your platform is not supported, some functionality might be unavailable.");
    }
    return prompt(&environment, "do: ", &evaluate);
}
