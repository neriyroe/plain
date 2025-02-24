/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     02/23/2013.
 * Revision 09/01/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

 enum {
    PLAIN_TYPE_BOOLEAN    = 0x01,
    PLAIN_TYPE_INTEGER    = 0x02,
    PLAIN_TYPE_KEYWORD    = 0x04,
    PLAIN_TYPE_LIST       = 0x05,
    PLAIN_TYPE_NIL        = 0x00,
    PLAIN_TYPE_POINTER    = 0x08,
    PLAIN_TYPE_REAL       = 0x06,
    PLAIN_TYPE_STRING     = 0x03,
    PLAIN_TYPE_SUBROUTINE = 0x07
 };
