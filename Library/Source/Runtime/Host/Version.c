/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     2013-11-09.
 * Revision 2026-03-29.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Version — see Runtime.h for the public contract.
 */

#include <Plain/Runtime.h>

PLAIN_WORD_DOUBLE PLAIN_VERSION(PLAIN_ENVIRONMENT* environment) {
    if(environment != NULL) {
        environment->meta.delimiters = "`\"'[{]},;: \t\r\n";
        environment->meta.version    = "2015.3";
    }
    return PLAIN_API;
}
