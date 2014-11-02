/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     02/23/2013,
 * Revision 10/31/2014,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

 enum {
    MOCOSEL_ERROR_SYNTAX                        = 0x100,
    MOCOSEL_ERROR_SYNTAX_ERRONEOUS_EXPRESSION   = 0x101,
    MOCOSEL_ERROR_SYNTAX_MISSING_BRACKET        = 0x102,
    MOCOSEL_ERROR_SYNTAX_MISSING_QUOTATION_MARK = 0x103,
    MOCOSEL_ERROR_SYNTAX_UNKNOWN_TOKEN          = 0x104,
    MOCOSEL_ERROR_SYSTEM                        = 0x200,
    MOCOSEL_ERROR_SYSTEM_WRONG_DATA             = 0x201
 };
