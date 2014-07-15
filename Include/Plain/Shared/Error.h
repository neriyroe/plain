/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     02/23/2013,
 * Revision 07/15/2014,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

 enum {
    MOCOSEL_ERROR_RUNTIME                       = 0x001,
    MOCOSEL_ERROR_RUNTIME_UNDEFINED_STATEMENT   = 0x002,
    MOCOSEL_ERROR_RUNTIME_WRONG_DATA            = 0x003,
    MOCOSEL_ERROR_SYNTAX                        = 0x100,
    MOCOSEL_ERROR_SYNTAX_ERRONEOUS_EXPRESSION   = 0x101,
    MOCOSEL_ERROR_SYNTAX_MISSING_BRACKET        = 0x102,
    MOCOSEL_ERROR_SYNTAX_MISSING_QUOTATION_MARK = 0x103,
    MOCOSEL_ERROR_SYNTAX_UNKNOWN_TOKEN          = 0x104,
    MOCOSEL_ERROR_SYSTEM                        = 0x200,
    MOCOSEL_ERROR_SYSTEM_WRONG_DATA             = 0x201
 };
