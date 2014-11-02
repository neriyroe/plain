/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/01/2014,
 * Revision 11/01/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#pragma once

int prompt(void* context, const char* identifier, void (*listener) (void*, const char*));
