/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     11/09/2013.
 * Revision 09/02/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 *
 * Evaluate — see Runtime.h for the public contract.
 */

#include <Plain/Runtime.h>

PLAIN_WORD_DOUBLE PLAIN_EVALUATE(struct PLAIN_CONTEXT* context, PLAIN_SUBROUTINE resolver, const PLAIN_BYTE* source, PLAIN_DELEGATE tracker, PLAIN_VALUE* value) {
    PLAIN_ASSERT(context != NULL);
    PLAIN_ASSERT(source != NULL);
    if(context == NULL || source == NULL) {
        return PLAIN_ERROR_SYSTEM_WRONG_DATA;
    }

    /* Wrap the source string in a segment (pointer pair). */
    struct PLAIN_SEGMENT body = {
        (PLAIN_BYTE*)source,
        (PLAIN_BYTE*)source + strlen((const char*)source)
    };
    if(body.from == body.to) {
        return 0;
    }

    /* Tokenize the source into a tree of PLAIN_LIST nodes. */
    struct PLAIN_LIST root;
    memset(&root, 0, sizeof(struct PLAIN_LIST));
    PLAIN_WORD_DOUBLE error = PLAIN_TOKENIZE(
        context,
        &root,
        NULL,
        (const PLAIN_BYTE*)context->environment.meta.delimiters,
        &body,
        tracker);

    /* Walk the tree — this is where statements are actually executed. */
    if(error == 0) {
        error = PLAIN_WALK(context, resolver, &root, value);
    }

    /* Free the parse tree. */
    if(root.layout.from != NULL || root.node != NULL) {
        PLAIN_UNLINK(&root);
    }

    return error;
}
