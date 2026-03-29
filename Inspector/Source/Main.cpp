/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     2026-03-22.
 * Revision 2026-03-29.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Inspector — interactive REPL for Plain.
 *
 * Creates a plain::Runtime with a "print" procedure bound, then enters
 * a read-eval loop via the platform-specific plain::prompt() (GNU readline
 * on Linux/macOS, standard I/O streams on Windows).
 */

#include <Plain/Runtime.hpp>
#include "Platform/Prompt.hpp"

#include <iostream>

int main() {
    plain::Runtime runtime;

    runtime.on_error([](const std::string& message) {
        std::cerr << message << '\n';
    });

    /* Register "print" — prints all arguments separated by spaces. */
    runtime.bind("print", [](const plain::Arguments& arguments) -> plain::Value {
        for(std::size_t index = 0; index < arguments.size(); ++index) {
            if(index > 0) std::cout << ' ';
            std::cout << arguments[index].as_string();
        }
        std::cout << '\n';
        return {};
    });

    return plain::prompt("do: ", [&runtime](const std::string& line) -> int {
        runtime.run(line);
        return 0;
    });
}
