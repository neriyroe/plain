/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     02/23/2013.
 * Revision 03/28/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Value — a tagged union representing any Plain value.
 *
 * Ownership rules:
 *   - length > 0:  data is heap-allocated (strings, integers, reals).
 *                  PLAIN_VALUE_CLEAR frees it; PLAIN_VALUE_COPY duplicates it.
 *   - length == 0: data is a marker or borrowed pointer (booleans, nil, objects).
 *                  Not freed by PLAIN_VALUE_CLEAR (except objects — see owner).
 *   - owner & PLAIN_OWNER_USER: data is a PLAIN_FRAME* managed by ref counting.
 *                  PLAIN_VALUE_CLEAR calls PLAIN_FRAME_RELEASE.
 *                  PLAIN_VALUE_COPY calls PLAIN_FRAME_RETAIN (shared ownership).
 */

struct PLAIN_VALUE {
    PLAIN_BYTE* data;            /* Payload — interpretation depends on type. */
    PLAIN_WORD_DOUBLE length;    /* Byte size of heap-allocated data; 0 for markers. */
    PLAIN_WORD_DOUBLE type;      /* One of PLAIN_TYPE_* (see Type.h). */
    PLAIN_WORD_DOUBLE owner;     /* Ownership flags (see PLAIN_OWNER_* below). */
};

/* Ownership flags for PLAIN_VALUE.owner. */
enum {
    PLAIN_OWNER_USER = 0x01  /* data is a PLAIN_FRAME* owned by the Plain runtime (ref-counted). */
};

/* Releases all resources owned by <value> and resets it to nil.
 * Object values (PLAIN_OWNER_USER) release their frame reference.
 * Heap-allocated data (length > 0) is freed. */
void PLAIN_VALUE_CLEAR(struct PLAIN_VALUE* value);

/* Deep-copies <source> into <destination>.
 * Object values get a shared reference (PLAIN_FRAME_RETAIN).
 * Heap-allocated data is duplicated into fresh memory. */
PLAIN_WORD_DOUBLE PLAIN_VALUE_COPY(struct PLAIN_VALUE* destination, const struct PLAIN_VALUE* source);

/* Returns nonzero if <value> is logically true:
 *   boolean  — true if data != NULL (yes)
 *   integer  — true if non-zero
 *   real     — true if non-zero
 *   string   — true if non-empty
 *   nil      — always false
 *   other    — true if data != NULL (objects, callables) */
PLAIN_WORD_DOUBLE PLAIN_VALUE_TRUTHY(const struct PLAIN_VALUE* value);
