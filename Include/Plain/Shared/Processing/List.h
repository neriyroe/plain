/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     02/23/2013.
 * Revision 09/01/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

struct PLAIN_LIST {
    struct PLAIN_SEGMENT keyword;
    struct PLAIN_SEGMENT layout;
    struct PLAIN_LIST* node;
    struct PLAIN_LIST* parent;
    struct PLAIN_SEGMENT segment;
};
