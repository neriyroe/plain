/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     03/28/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Framework — the Plain standard library.
 * Provides PLAIN_FRAMEWORK_REGISTER, which installs all built-in procedures
 * (if, repeat, define, set, arithmetic operators, etc.) into the root frame.
 * Include this alongside <Plain/Runtime.h> if you want the standard library;
 * omit it for a bare runtime with only host-supplied callables.
 */

#pragma once

#include <Plain/Runtime.h>



/* Registers all standard built-in procedures in the root frame of <context>.
 * Call once after PLAIN_FRAME_CREATE, before any evaluation. Built-ins are
 * mutable bindings — Plain code may freely override any of them. */
PLAIN_WORD_DOUBLE PLAIN_FRAMEWORK_REGISTER(struct PLAIN_CONTEXT* context);
