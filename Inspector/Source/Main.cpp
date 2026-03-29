/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     03/22/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Inspector — Plain REPL using the C++ interface.
 * See Samples/ for extended demos (object bridge, class binding, etc.).
 */

#include <Plain/Runtime.hpp>

#include <iostream>
#include <string>

extern "C" int prompt(void* context, const char* identifier,
                      int (*listener)(void*, const char*));

static int evaluate(void* context, const char* source) {
    reinterpret_cast<plain::Runtime*>(context)->run(source);
    return 0;
}

int main() {
    plain::Runtime runtime;

    runtime.on_error([](const std::string& message) {
        std::cerr << message << '\n';
    });

    runtime.bind("print", [](const plain::Arguments& arguments) -> plain::Value {
        for(size_t index = 0; index < arguments.size(); index++) {
            if(index > 0) std::cout << ' ';
            std::cout << arguments[index].as_string();
        }
        std::cout << '\n';
        return {};
    });

    return prompt(&runtime, "do: ", &evaluate);
}
