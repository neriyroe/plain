/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     03/22/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Inspector — interactive REPL for Plain.
 *
 * Creates a plain::Runtime with a "print" procedure bound, then enters
 * a read-eval loop via the platform-specific prompt() function (readline
 * on Linux/macOS, fgets on Windows).
 */

#include <Plain/Runtime.hpp>

#include <iostream>
#include <string>

/* Platform-specific prompt function (see GNU/Prompt.c, Windows/Prompt.c). */
extern "C" int prompt(void* context, const char* identifier,
                      int (*listener)(void*, const char*));

/* Callback invoked by prompt() for each line of input.
 * Evaluates the line as Plain source code. */
static int evaluate(void* context, const char* source) {
    reinterpret_cast<plain::Runtime*>(context)->run(source);
    return 0;
}

int main() {
    plain::Runtime runtime;

    /* Report errors to stderr. */
    runtime.on_error([](const std::string& message) {
        std::cerr << message << '\n';
    });

    /* Register "print" — the only I/O procedure.
     * Prints all arguments separated by spaces, followed by a newline. */
    runtime.bind("print", [](const plain::Arguments& arguments) -> plain::Value {
        for(size_t index = 0; index < arguments.size(); index++) {
            if(index > 0) {
                std::cout << ' ';
            }
            std::cout << arguments[index].as_string();
        }
        std::cout << '\n';
        return {};
    });

    /* Enter the read-eval loop. */
    return prompt(&runtime, "do: ", &evaluate);
}
