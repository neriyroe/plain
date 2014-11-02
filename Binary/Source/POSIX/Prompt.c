/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/01/2014,
 * Revision 11/02/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include <readline/readline.h>
#include <readline/history.h>
#include "../Environment.h"

int prompt(void* context, const char* identifier, void (*listener) (void*, const char*)) {
    char* line = NULL;
    while(line = readline(identifier)) {
        if(*line != '\0') {
            listener(context, line);
        }
        add_history(line);
        free(line);
    }
    return 0;
}
