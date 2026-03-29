/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     02/23/2013.
 * Revision 09/02/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 *
 * Error — error codes returned by runtime and parser functions.
 *
 * All functions return 0 on success.  Non-zero values are either errors
 * (0x1xx for syntax, 0x2xx for system) or control-flow signals (defined
 * in Frame.h: PLAIN_SIGNAL_BREAK, PLAIN_SIGNAL_RETURN, etc.).
 */

enum {
    /* Syntax errors — reported by the tokenizer via the tracker delegate. */
    PLAIN_ERROR_SYNTAX                        = 0x100,
    PLAIN_ERROR_SYNTAX_ERRONEOUS_EXPRESSION   = 0x101,  /* Malformed number or expression. */
    PLAIN_ERROR_SYNTAX_MISSING_BRACKET        = 0x102,  /* Unmatched [ or {. */
    PLAIN_ERROR_SYNTAX_MISSING_QUOTATION_MARK = 0x103,  /* Unterminated string literal. */
    PLAIN_ERROR_SYNTAX_UNKNOWN_TOKEN          = 0x104,  /* Unrecognizable token. */

    /* System errors — allocation failures and invalid arguments. */
    PLAIN_ERROR_SYSTEM                        = 0x200,  /* Memory allocation failed. */
    PLAIN_ERROR_SYSTEM_WRONG_DATA             = 0x201   /* NULL or invalid argument. */
};
