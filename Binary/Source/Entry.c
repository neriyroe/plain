/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/01/2014,
 * Revision 11/01/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include "Application.h"
#include "Binding.h"

int main() {
    MOCOSEL_ENVIRONMENT environment;
    if(MOCOSEL_VERSION(&environment) == 0) {
        printf("Warning! Your platform is not supported, some functionality might be unavailable.");
    }
    return prompt(&environment, "do: ", &run);
}
