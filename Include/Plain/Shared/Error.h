/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     02/23/2013.
 * Revision 09/02/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

 enum {
    PLAIN_ERROR_SYNTAX                        = 0x100,
    PLAIN_ERROR_SYNTAX_ERRONEOUS_EXPRESSION   = 0x101,
    PLAIN_ERROR_SYNTAX_MISSING_BRACKET        = 0x102,
    PLAIN_ERROR_SYNTAX_MISSING_QUOTATION_MARK = 0x103,
    PLAIN_ERROR_SYNTAX_UNKNOWN_TOKEN          = 0x104,
    PLAIN_ERROR_SYSTEM                        = 0x200,
    PLAIN_ERROR_SYSTEM_WRONG_DATA             = 0x201
 };
