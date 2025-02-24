/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     11/09/2013.
 * Revision 09/02/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

#include <Plain/VM.h>

PLAIN_WORD_DOUBLE PLAIN_VERSION(PLAIN_ENVIRONMENT* environment) {
    if(environment != NULL) {
        environment->meta.pattern = "`\"'[{]},;: \t\r\n";
        environment->meta.version = "2015.3";
    }
    return PLAIN_API;
}
