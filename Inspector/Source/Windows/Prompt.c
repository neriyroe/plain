/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     2026-03-22.
 * Revision 2026-03-29.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 */

#include <stdio.h>
#include <string.h>

int prompt(void* context, const char* identifier, int (*listener)(void*, const char*)) {
    char line[4096];
    int code = 0;
    for(;;) {
        printf("%s", identifier);
        fflush(stdout);
        if(fgets(line, sizeof(line), stdin) == NULL) {
            break;
        }
        /* Strip trailing newline. */
        size_t length = strlen(line);
        while(length > 0 && (line[length - 1] == '\n' || line[length - 1] == '\r')) {
            line[--length] = '\0';
        }
        if(length > 0) {
            code = listener(context, line);
        }
        if(code != 0) {
            break;
        }
    }
    return code;
}
