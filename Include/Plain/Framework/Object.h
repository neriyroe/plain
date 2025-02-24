/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     10/11/2013.
 * Revision 09/01/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

struct PLAIN_OBJECT {
    struct {
        struct PLAIN_SEGMENT data;
        struct PLAIN_LIST structure;
    } segment;
};
