/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     11/09/2013.
 * Revision 09/02/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

#include <Plain/Runtime.h>

PLAIN_WORD_DOUBLE PLAIN_EVALUATE(PLAIN_ENVIRONMENT* environment, PLAIN_SUBROUTINE function, const PLAIN_BYTE* source, PLAIN_DELEGATE tracker, PLAIN_VALUE* value) {
    PLAIN_ASSERT(environment != NULL);
    PLAIN_ASSERT(source != NULL);
    /* PLAIN_ERROR_SYSTEM_WRONG_DATA. */
    if(environment == NULL || source == NULL) {
        return PLAIN_ERROR_SYSTEM_WRONG_DATA;
    }
    PLAIN_SEGMENT body = { (PLAIN_BYTE*)source, (PLAIN_BYTE*)source + strlen((const char*)source) };
    PLAIN_LIST root = { NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0 };
    if(object.segment.data.from == object.segment.data.to) {
        return 0;
    }
    /* System failures and syntax errors, <tracker> shall be able to print a nice report. */
    PLAIN_WORD_DOUBLE error = PLAIN_TOKENIZE(environment,
                                             &root,
                                             NULL,
                                             (const PLAIN_BYTE*)environment->meta.delimiters,
                                             &body,
                                             tracker);
    /* Only system failures, no runtime dependencies. */
    if(error == 0) {
       error = PLAIN_WALK(environment, function, &root, value);
    }
    /* The object might be dummy. */
    if(root.layout.from != NULL || root.node != NULL) {
        PLAIN_UNLINK(&root);
    }
    return error;
}
