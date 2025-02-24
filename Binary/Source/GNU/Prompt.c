/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     11/01/2014.
 * Revision 08/30/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

int prompt(void* context, const char* identifier, int (*listener) (void*, const char*)) {
    int code = 0;
    for(;;) {
        char* line = readline(identifier);
        if(*line != '\0') {
            code = listener(context, line);
        }
        add_history(line);
        free(line);
        if(code != 0) {
            break;
        }
    }
    return code;
}
