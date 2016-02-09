/*
 * Author   Neriy Roe <nr@mocosel.com>.
 * Date     10/15/2014.
 * Revision 09/01/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

/* Syntax delegate to analyse corresponding errors. */
typedef void (*MOCOSEL_DELEGATE) (void* context, const MOCOSEL_BYTE* data, MOCOSEL_WORD_DOUBLE length, MOCOSEL_WORD_DOUBLE type);
