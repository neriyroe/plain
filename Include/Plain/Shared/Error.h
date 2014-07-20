/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     02/23/2013,
 * Revision 07/20/2014,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

 enum {
    MOCOSEL_ERROR_RUNTIME                       = 0x100,
    MOCOSEL_ERROR_RUNTIME_UNDEFINED_STATEMENT   = 0x101,
    MOCOSEL_ERROR_RUNTIME_WRONG_DATA            = 0x102,
    MOCOSEL_ERROR_SYNTAX                        = 0x200,
    MOCOSEL_ERROR_SYNTAX_ERRONEOUS_EXPRESSION   = 0x201,
    MOCOSEL_ERROR_SYNTAX_MISSING_BRACKET        = 0x202,
    MOCOSEL_ERROR_SYNTAX_MISSING_QUOTATION_MARK = 0x203,
    MOCOSEL_ERROR_SYNTAX_UNKNOWN_TOKEN          = 0x204,
    MOCOSEL_ERROR_SYSTEM                        = 0x400,
    MOCOSEL_ERROR_SYSTEM_WRONG_DATA             = 0x401
 };
