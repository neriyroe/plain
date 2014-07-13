#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <Plain/VM.h>

struct MOCOSEL_VALUE* call(MOCOSEL_CONTEXT* context, const struct MOCOSEL_SEGMENT* segment) {
    printf("Calling %.*s...\n", segment.to - segment.from, (char*)segment.from);
    return NULL;
}

int main() {
    struct MOCOSEL_ENVIRONMENT environment;
    if(MOCOSEL_VERSION(&environment) == 0) {
        printf("Unsupported platform.\n");
    }
    for(;;) {
        char* line = readline("local: ");
        if(line == NULL) {
            break;
        }
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_RUN(NULL, &environment, MOCOSEL_SEGMENT_COMPILE | MOCOSEL_SEGMENT_EXECUTE, &call, (struct MOCOSEL_OBJECT*)alloca(sizeof(struct MOCOSEL_OBJECT)), line);
        if(error != 0) {
            const char* category = "Unknown";
            if(error & MOCOSEL_ERROR_RUNTIME) {
                category = "Runtime";
            }
            if(error & MOCOSEL_ERROR_SYNTAX) {
                category = "Syntax";
            }
            if(error & MOCOSEL_ERROR_SYSTEM) {
                category = "System";
            }
            printf("%s error %d.\n", category, error);
        }
        free(line);
    }
    return 0;
}
